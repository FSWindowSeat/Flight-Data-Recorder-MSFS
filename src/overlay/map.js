
// Global vars
var gMap, gMarker, gLine, lat, lon, alt, hdg;
var lineNo = 0, cycleNo = 0;
var updFreq = 0.25, cycleLn = 0.02, speed = 2, processingTms = 10;
var zoomReg = 8, zoomGlob = 4, zoomAdj = 0, zoomCycleReg = 60, zoomCycleGlob = 30, zoomCycleMax = 30, zoomInterv = 0;
var totFltTime = 0, tEl = 0, remainFltTime = 0, gmtOffset = 0, fltTimeInfoCycle = 20, fltTimeInfoInterv = 0;
var dst = 0, spd = 0, zul = 0, fltDataCycle = 10, fltDataInterv = 0;
var randSpdDev = 0, randAltDev = 0, randHdgDev = 0, randTimeDev = 0;
var reader = new FileReader();
var icon;
var linePath = [];
var depDate = new Date(Date.UTC(2020, 0, 1, 0, 0, 0));
var depGMTSec = 0;
var destDate = new Date(Date.UTC(2020, 0, 1, 0, 0, 0));
var destGMTSec = 0;

window.onload = function () {

  // Get data from SimConnect REST end point and update overlay, i.e. map and data
  callRestEndPoint();
  updateMap();
  updateData();

  cycleNo++;
}

function callRestEndPoint() {
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (xhttp.readyState == 4 && xhttp.status == 200) {
        // Process JSON
        var jsonResponse = JSON.parse(xhttp.responseText);

        document.getElementById('labelTimeTo').textContent = "Time to " + jsonResponse["destName"];

        depDate.setUTCHours(jsonResponse["depHH"]);
        depDate.setUTCMinutes(jsonResponse["depMM"]);

        depGMTSec = jsonResponse["depGMTHH"] * 3600;
        if (depGMTSec > 0)
          depGMTSec += jsonResponse["depGMTMM"] * 60;
        else
          depGMTSec -= jsonResponse["depGMTMM"] * 60;

        destDate.setUTCHours(jsonResponse["fltHH"]);
        destDate.setUTCMinutes(jsonResponse["fltMM"]);

        destGMTSec = jsonResponse["destGMTHH"] * 3600;
        if (depGMTSec > 0)
          destGMTSec += jsonResponse["destGMTMM"] * 60;
        else
          destGMTSec -= jsonResponse["destGMTMM"] * 60;

        lat = jsonResponse["lat"];
        lon = jsonResponse["lon"];
        alt = jsonResponse["alt"];
        hdg = jsonResponse["hdg"];
        spd = jsonResponse["spd"];
        zul = jsonResponse["zul"];
        dst = jsonResponse["dst"];
        tEl = jsonResponse["tEl"];
      }
    };

    xhttp.open("GET", "http://127.0.0.1:1234", true);
    xhttp.send();
  }, updFreq * 1000);
}


function initMap() {

  gMap = new google.maps.Map(document.getElementById("map"), {
    zoom: 12,
    center: { lat: 47.448902, lng: -122.304318 },
    disableDefaultUI: true,
    mapTypeId: 'hybrid'
  });

  icon = {
    path: "M134.875,19.74c0.04-22.771,34.363-22.771,34.34,0.642v95.563L303,196.354v35.306l-133.144-43.821v71.424l30.813,24.072 v27.923l-47.501-14.764l-47.501,14.764v-27.923l30.491-24.072v-71.424L3,231.66v-35.306l131.875-80.409V19.74z",
    fillColor: '#fcfcfc',
    fillOpacity: .98,
    strokeWeight: 5,
    strokeColor: '#3a3a3a',
    scale: 0.2,
    rotation: 0,
    anchor: new google.maps.Point(152, 156)
  }

  gMarker = new google.maps.Marker({
    position: gMap.getCenter(),
    icon: icon,
    draggable: true,
    map: gMap
  });

  gLine = new google.maps.Polyline({
    path: linePath,
    //Red strokeColor: "#ec1818",
    strokeColor: "#76c80a",
    strokeOpacity: .90,
    strokeWeight: 10,
    geodesic: true,
    map: gMap
  });

}

function updateMap() {
  setInterval(function () {
    if (lat && lon) {
      // Update map position
      ctrMap = new google.maps.LatLng(lat, lon);
      gMap.setCenter(ctrMap);

      // Update zoom level
      if (parseFloat(alt) <= 4000.0)
        zoomAdj = 7;
      else if (parseFloat(alt) > 4000.0 && parseFloat(alt) <= 10000.0)
        zoomAdj = 4;
      else
        zoomAdj = 0;

      if (zoomInterv <= zoomCycleReg)
        gMap.setZoom(8 + zoomAdj);
      else if (zoomInterv > zoomCycleReg && zoomInterv <= (zoomCycleReg + zoomCycleGlob))
        gMap.setZoom(5 + zoomAdj);
      else if (parseFloat(alt) > 4000.0 && zoomInterv > (zoomCycleReg + zoomCycleGlob) && zoomInterv <= (zoomCycleReg + zoomCycleGlob + zoomCycleMax))
        gMap.setZoom(2 + zoomAdj);
      else
        zoomInterv = 0;

      zoomInterv += updFreq;

      // Update marker position and rotation
      gMarker.setPosition(ctrMap);
      icon.rotation = parseFloat(hdg);
      gMarker.setOptions({ icon: icon });

      // Update progress line
      linePath.push(new google.maps.LatLng(parseFloat(lat), parseFloat(lon)));
      gLine.setPath(linePath);
    }

  }, updFreq * 1000);
}

function updateData() {
  setInterval(function () {

    // Update time to destination information
    //gmtOffset = (document.getElementById('destGMT').value - document.getElementById('depGMT').value) * 3600;
    gmtOffset = destGMTSec - depGMTSec;
    totFltTime = Math.abs(destDate.getUTCHours() * 3600 + destDate.getUTCMinutes() * 60);
    remainFltTime = totFltTime - tEl;

    var hours = Math.floor(remainFltTime / 3600);
    remainFltTime %= 3600;
    var minutes = Math.floor(remainFltTime / 60);

    document.getElementById('valueTimeTo').textContent = hours + " h " + pad(minutes, 2) + " m";

    // Update time information
    var depLocalTime = dateAdd(depDate, 'second', tEl);
    var destLocalTime = dateAdd(depDate, 'second', tEl + gmtOffset);
    var destArrTime = dateAdd(depDate, 'second', totFltTime + randTimeDev + gmtOffset);

    if (fltTimeInfoInterv <= fltTimeInfoCycle) {
      // Time of Arrival        
      document.getElementById('labelTimeAt').textContent = "Time of Arrival";
      document.getElementById('valueTimeAt').textContent = pad(destArrTime.getUTCHours(), 2) + ":" + pad(destArrTime.getUTCMinutes(), 2);
    } else if (fltTimeInfoInterv > fltTimeInfoCycle && fltTimeInfoInterv <= (fltTimeInfoCycle * 2)) {
      // Local Time at Destination
      document.getElementById('labelTimeAt').textContent = "Time at Destination";
      document.getElementById('valueTimeAt').textContent = pad(destLocalTime.getUTCHours(), 2) + ":" + pad(destLocalTime.getUTCMinutes(), 2);
    } else if (fltTimeInfoInterv > (fltTimeInfoCycle * 2) && fltTimeInfoInterv <= (fltTimeInfoCycle * 3)) {
      // Local Time at Departure
      document.getElementById('labelTimeAt').textContent = "Time at Origin";
      document.getElementById('valueTimeAt').textContent = pad(depLocalTime.getUTCHours(), 2) + ":" + pad(depLocalTime.getUTCMinutes(), 2);
    } else {
      fltTimeInfoInterv = 0;
    }
    fltTimeInfoInterv += updFreq * speed;

    // Update distance traveled
    document.getElementById('valueDistTraveled').textContent = Math.round(dst) + ' miles';

    // Update flight data: Alternate altitude, speed and heading vars
    if (fltDataInterv <= fltDataCycle) {
      // Altitude
      document.getElementById('labelFltData').textContent = 'Altitude';
      document.getElementById('valueFltData').textContent = Math.round(alt * 3.28084) + randAltDev + ' ft';
    } else if (fltDataInterv > fltDataCycle && fltDataInterv <= (fltDataCycle * 2)) {
      // Speed
      document.getElementById('labelFltData').textContent = 'Ground Speed';
      if (spd <= 1) {
        document.getElementById('valueFltData').textContent = '0 kts';
      } else {
        document.getElementById('valueFltData').textContent = Math.round(spd) + randSpdDev + ' kts';
      }
    } else if (fltDataInterv > (fltDataCycle * 2) && fltDataInterv <= (fltDataCycle * 3)) {
      // Heading
      document.getElementById('labelFltData').textContent = 'Heading';
      document.getElementById('valueFltData').textContent = Math.round(hdg) + randHdgDev + ' deg';
    } else {
      // Update randoms deviations each cycle
      /*
      if(lineVals[24] == 3){
          randSpdDev = 0;//getRandomInt(-5,5); 
          randAltDev = 0;//getRandomInt(-10, 10); 
          if(lineVals[3] > 2 && lineVals[3] < 358)    
              randHdgDev = 0;//getRandomInt(-1,1); 
          randTimeDev = 0;//getRandomInt(-1,1) * 60;
      }*/
      fltDataInterv = 0;
    }
    fltDataInterv += updFreq;
  }, updFreq * 1000);
}

function changeDateTimeZone(date, timeZoneOffset) {
  date.setTime(date.getTime() + date.getTimezoneOffset() * 60 * 1000 + timeZoneOffset * 60 * 60 * 1000);
}

function getCsvValuesFromLine(line) {
  var values = line.split(',');
  value = values.map(function (value) {
    return value.replace(/\"/g, '');
  });
  return values;
}

function getRandomInt(min, max) {
  min = Math.ceil(min);
  max = Math.floor(max);
  return Math.floor(Math.random() * (max - min)) + min;
}

/**************************************************************************************************
 * Zero padding. Adds leading zeros to a number and returns a string.
 * Note: This assumes you'd never want more than 10 digits
 * 
 * @param num       Number
 * @param size      Total number of digits
 **************************************************************************************************/
function pad(num, size) {
  var s = "000000000" + num;
  return s.substr(s.length - size);
}


/**************************************************************************************************
 * Add time to a date. Modeled after MySQL DATE_ADD function.
 * Example: dateAdd(new Date(), 'minute', 30)  //returns 30 minutes from now.
 * 
 * @param date      Date to start with
 * @param interval  One of: year, quarter, month, week, day, hour, minute, second
 * @param units     Number of units of the given interval to add.
 **************************************************************************************************/
function dateAdd(date, interval, units) {
  if (!(date instanceof Date))
    return undefined;

  var ret = new Date(date); //don't change original date

  var checkRollover = function () { if (ret.getDate() != date.getDate()) ret.setDate(0); };

  switch (String(interval).toLowerCase()) {
    case 'year': ret.setFullYear(ret.getFullYear() + units); checkRollover(); break;
    case 'quarter': ret.setMonth(ret.getMonth() + 3 * units); checkRollover(); break;
    case 'month': ret.setMonth(ret.getMonth() + units); checkRollover(); break;
    case 'week': ret.setDate(ret.getDate() + 7 * units); break;
    case 'day': ret.setDate(ret.getDate() + units); break;
    case 'hour': ret.setTime(ret.getTime() + units * 3600000); break;
    case 'minute': ret.setTime(ret.getTime() + units * 60000); break;
    case 'second': ret.setTime(ret.getTime() + units * 1000); break;
    default: ret = undefined; break;
  }
  return ret;
}