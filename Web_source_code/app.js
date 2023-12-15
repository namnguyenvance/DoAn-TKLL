const express = require('express');
const bodyParser = require('body-parser');
const mongoose = require('mongoose')
const path = require('path');
const mainroute = require('./routes/main.js');
const authroute = require('./routes/auth.js');
const session = require('express-session');
const MongoDBStore = require('connect-mongodb-session')(session);
const moment = require('moment-timezone').tz.setDefault('Asia/Ho_Chi_Minh')
const csrf = require('csurf')
const flash = require('connect-flash')

const errorController = require('./controllers/error');
const DATA = require('./firebase/config.js');
const User = DATA.doc('data');
const Door = DATA.doc('Door');
const Pump = DATA.doc('Pump');
const Light = DATA.doc('Light');

const app = express();
const sensors = require('./models/sensors.js');
const hist_door = require('./models/door.js')
const schedule_time = require('./models/time-schedule.js')

const MONGODB_URI = 'mongodb+srv://hoang:hoang0965766364@cluster0.ambcnxe.mongodb.net/session-store?retryWrites=true&w=majority&appName=AtlasApp'

const store = new MongoDBStore({
  uri: MONGODB_URI,
  collection: 'sessions',
});

const csrfProtection = csrf();
app.use(flash())

// Sử dụng EJS làm view engine
app.set('view engine', 'ejs');
app.set('views', 'views')

app.use(express.static(path.join(__dirname, 'public')));
app.use(bodyParser.urlencoded({ extended: false }));
app.use(express.json())
app.use(session({secret: 'my secret', resave: false, saveUninitialized: false, store: store}))
// Route để hiển thị nhiệt độ và độ ẩm
app.use(csrfProtection)
app.use(mainroute);
app.use(authroute)

app.use(errorController.get404);


mongoose.connect(MONGODB_URI).then(result=>{  
  const server = app.listen(3000, () => {
  console.log(`Server is running on port 3000`);
  const io = require('socket.io')(server);
  setInterval(async () => {
    const currentDate = moment();
    const result = await schedule_time.find();
    const data = await Pump.get(); 
    for (let items of result) {
        const currentHour = currentDate.hour();
        const currentMinute = currentDate.minute();

        if (items['hour'] == currentHour && items['minute'] == currentMinute) {
            if (items['check']) {
                Pump.update({ 'Pump': true });
                await schedule_time.findByIdAndUpdate(items['_id'], { 'check': false });
            }
        }
        if (!data.data().Pump && !items['check']) {
          await schedule_time.findByIdAndUpdate(items['_id'], { 'process': false });
        }
        const hour_end = (items['hour'] + Math.floor((items['minute'] + items['intervals']) / 60)) % 24;
        const minute_end = (items['minute'] + items['intervals']) % 60;

        if (hour_end == currentHour && minute_end == currentMinute && !items['check']) {
            if (items['process']) {
              Pump.update({ 'Pump': false });
              await schedule_time.findByIdAndUpdate(items['_id'], { 'check': true, 'process': true });
            }
            else{
              await schedule_time.findByIdAndUpdate(items['_id'], { 'check': true, 'process': true });
            }
        }
    }
  }, 2000);
  setInterval(async ()=>{
    const doc = await User.get() 
    if(doc.data().soil_auto_mode){
      if(doc.data().Soil > doc.data().Highest_soil){
        await Pump.update({
          Pump: false
        })
      }
      if(doc.data().Soil <= doc.data().Lowest_soil){
        await Pump.update({
          Pump: true
        })
      }
    }
    if(doc.data().light_auto_mode){
      if(doc.data().Brightness > doc.data().Highest_brightness){
        await Light.update({
          Light: false
        })
      }
      if(doc.data().Brightness <= doc.data().Lowest_brightness){
        await Light.update({
          Light: true
        })
      }
    }
  }, 
  1000
  )
  Door.onSnapshot(async(snapshot)=>{
    snapshot = await Door.get()
    var data = snapshot.data()  
    if(data){
      const door = data["Door"]
      const last_access_door = await hist_door.find().sort({createdAt:-1}).limit(1)
      if(!last_access_door) {
        const access_door = new hist_door({
          state: door,
          message: door ? "The door is opened" : "The door is closed" 
        })
        return access_door.save().then(
          result=>{
            try {
              io.emit('data-door-update');
            } catch (error) {
              console.log(error)
            }
          }
        )
      }
      else{
        if(last_access_door[0]['state'] != door){
          console.log(last_access_door)
          const access_door = new hist_door({
            state: door,
            message: door ? "The door is opened" : "The door is closed" 
          })
          return access_door.save().then(
            result=>{
              try {
                io.emit('data-door-update');
              } catch (error) {
                console.log(error)
              }
            }
          )
        }else{
          try {
            io.emit('data-door-update');
          } catch (error) {
            console.log(error)
          }
        }
      }
    }
  })
  Pump.onSnapshot(async(snapshot) => {
    io.emit('data-pump-update');
  })
  Light.onSnapshot(async(snapshot) => {
    io.emit('data-light-update');
  })
  //Lắng nghe sự kiện thay đổi trong Firebase
  User.onSnapshot(async(snapshot) => {
    snapshot = await User.get()
    var data = snapshot.data()  
    console.log(data)
    if (data) {
      const temperature = data["Temperature"];
      const humidity = data["Humidity"];
      const brightness = data["Brightness"]
      const soil = data['Soil']
      io.emit('data-update', { temperature, humidity,  brightness,soil});
    }
  });

  io.on('connection', socket => {
    console.log('A user connected');
    // Lắng nghe sự kiện khi dữ liệu cần được cập nhật
    socket.on('get-latest-data', () => {
      // Truyền dữ liệu mới tới trình duyệt khi cần
      io.emit('data-update', { temperature, humidity, brightness, soil});
    });
  });
  });
}).catch(err=>{
  console.log(err)
})

