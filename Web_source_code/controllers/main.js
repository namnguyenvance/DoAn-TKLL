const User = require('../firebase/config.js').doc('data');
const Door = require('../firebase/config.js').doc('Door');
const Pump = require('../firebase/config.js').doc('Pump');
const Lightxx = require('../firebase/config.js').doc('Light');
exports.maintheme = async(req, res) => {
  const snapshot = await User.get()
  const Doorx = await Door.get()
  const Pumpx = await Pump.get()
  const Lightx = await Lightxx.get()
  var list = snapshot.data()
  console.log(list)
  temperature = list["Temperature"]
  humidity = list["Humidity"]
  pump = Pumpx.data().Pump
  door = Doorx.data().Door
  brightness = list["Brightness"]
  soil = list['Soil']
  Light = Lightx.data().Light
  
  res.render('iot', { temperature, humidity, pump, door, brightness,Light,soil,
  pageTitle: "IOT Garden", isAuthenticated: req.session.isLoggedIn, csrfToken: req.csrfToken()});
}
  