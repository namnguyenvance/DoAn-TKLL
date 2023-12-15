const express = require('express');
const router = express.Router();
const authController = require('../controllers/auth.js')
const isauth = require('../middleware/is-auth.js')

router.get('/login', authController.getLogin);

router.get('/signup', authController.getSignup);

router.post('/login', authController.postLogin);

router.post('/signup', authController.postSignup);

router.post('/logout', authController.postLogout);

router.get('/history_access', isauth , authController.getHistoryAccess);
router.get('/history-door', isauth , authController.getHistoryDoor);
router.get('/pump-shedule', isauth , authController.getPumpShedule);
router.post('/pump-shedule', isauth , authController.postPumpShedule);
router.post('/pump-shedule-delete', isauth , authController.deletePumpShedule);



router.get('/chart-temperature', isauth, authController.getChartTemp);
router.get('/chart-humidity', isauth, authController.getChartHumidity);
router.get('/chart-brightness', isauth, authController.getChartBrightness);
router.post('/chart-brightness', isauth, authController.postChartBrightness);

router.get('/chart-soil-moisture', isauth, authController.getChartSoilMoisture);
router.post('/chart-soil-moisture', isauth, authController.postChartSoilMoisture);




module.exports = router


