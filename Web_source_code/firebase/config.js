const firebase = require('firebase');
const firebaseConfig = {
  apiKey: "AIzaSyCOYDfXX2g7eQ2q35H1AhrCrC9s_MedTIo",
  authDomain: "tkll-project-402e9.firebaseapp.com",
  databaseURL: "https://tkll-project-402e9-default-rtdb.firebaseio.com",
  projectId: "tkll-project-402e9",
  storageBucket: "tkll-project-402e9.appspot.com",
  messagingSenderId: "440382764725",
  appId: "1:440382764725:web:36a2190a7d05f9bf8bb0fc",
  measurementId: "G-QSDYXY6YP0"
};
firebase.initializeApp(firebaseConfig)
const db = firebase.firestore()
const User = db.collection('User')
module.exports = User 
