const mongoose = require('mongoose');
const Schema = mongoose.Schema;
const time_schedule = new Schema({
  hour: {
    type: Number,
    required: true
  },
  minute: {
    type: Number,
    required: true
  },
  intervals:{
    type: Number,
    require: true
  },
  check:{
    type: Boolean,
    required: true
  },
  process:{
    type: Boolean,
    required: true
  }
});
module.exports = mongoose.model('time_schedule', time_schedule);