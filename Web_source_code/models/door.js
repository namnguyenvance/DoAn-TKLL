const mongoose = require('mongoose');
const Schema = mongoose.Schema;
const history_door = new Schema({
  state: {
    type: Boolean,
    required: true
  },
  message:{
    type: String,
    required: true
  }
}, {timestamps: true});
module.exports = mongoose.model('history_door', history_door);