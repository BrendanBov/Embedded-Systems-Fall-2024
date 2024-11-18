

if (!!window.EventSource) {
    var source = new EventSource('/');
    source.onmessage = function(e) {
      if (e.data.length < 6) return;
      var bumper = e.data[1];
      var cliff = e.data[3];
      var drop = e.data[5];

      document.getElementById("but1").value = SensorRCL(bumper);
      document.getElementById("but2").value = SensorRCL(cliff);
      document.getElementById("but3").value = SensorDrop(drop);

      /*
      document.getElementById("but1").value = "But1";
      document.getElementById("but2").value = "But2";
      document.getElementById("but3").value = "But3";
      */
    }
}

function SensorRCL(sensorData)
{
  switch(sensorData)
  {
    case "0": return "OFF"; 
    case "1": return "Right"; 
    case "2": return "Center"; 
    case "3": return "Right/Center"; 
    case "4": return "Left"; 
    case "5": return "Right/Left";
    case "6": return "Center/Left";
    case "7": return "Right/Center/Left";
    default: return "OFF";
  }
}

function SensorDrop(sensorData)
{
  switch(sensorData)
  {
    case "0": return "OFF"; 
    case "1": return "Right"; 
    case "2": return "Left"; 
    case "3": return "Right/Left"; 
    default: return "OFF";
  }
}