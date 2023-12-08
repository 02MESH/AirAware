const firebaseConfig = {
    apiKey: "AIzaSyD3kkHqfGoEcl-njGv686743J9NfCHIrU4",
    authDomain: "airaware-d3ab4.firebaseapp.com",
    databaseURL: "https://airaware-d3ab4-default-rtdb.europe-west1.firebasedatabase.app",
    projectId: "airaware-d3ab4",
    storageBucket: "airaware-d3ab4.appspot.com",
    messagingSenderId: "410097835898",
    appId: "1:410097835898:web:820c1e66884c0f752200c3"
};

firebase.initializeApp(firebaseConfig);
var db = firebase.database();

async function retrieveData() {
    try {
        var temperatureDocument = db.ref('SensorValues/Temperature');
        var humidityDocument = db.ref('SensorValues/Humidity');
        
        var temperatureValue = 0, humidityValue = 0;
        
        
        temperatureDocument.on('value', function(snapshot) {
            var data = snapshot.val();
            temperatureValue = data;
        });
        humidityDocument.on('value', function(snapshot) {
            var data = snapshot.val();
            humidityValue = data;
        });
        
        let jsonData = {
            temperature: temperatureValue,
            humidity: humidityValue
        }
        
        return jsonData;
    } catch (error) {
        console.error('Error fetching data: ', error);
        return null;
    }
}

async function getTime() {
    const currentDate = new Date();
    
    const currentHour = currentDate.getHours();
    const currentMinute = currentDate.getMinutes();
    const currentSecond = currentDate.getSeconds();
    
    const time = `${currentHour}:${currentMinute}:${currentSecond}`;

    return time;
}

async function updateChart() {
    const data = await retrieveData();
    const time = await getTime();
    
    const temperatureCanvas = document.getElementById('temperatureChart');
    const humidityCanvas = document.getElementById('humidityChart');
    
    const temperatureChart = new Chart(temperatureCanvas, {
        type: 'line', 
        data: {
            labels: [0],
            datasets: [{
                label: 'Temperature (in °C)',
                data: [10],
                borderColor: 'red',
                fill: false
            }]
        },
        options: {}
    });
    
    const humidityChart = new Chart(humidityCanvas, {
        type: 'line',
        data: {
            labels: [0],
            datasets: [{
                label: "Humidity (in %)",
                data: [10],
                borderColor: 'blue',
                fill: false
            }]
        },
        options: {}
    });

    setInterval(async() => {
        const updatedData = await retrieveData();
        const time = await getTime();

        temperatureChart.data.labels.push(time);
        humidityChart.data.labels.push(time);

        // temperatureChart.data.labels = updatedData.timeLabels;
        temperatureChart.data.datasets[0].data.push(updatedData.humidity);

        // humidityChart.data.labels = updatedData.timeLabels;
        humidityChart.data.datasets[0].data.push(updatedData.temperature);

        temperatureChart.update();
        humidityChart.update();
    }, 15000);
}

function getData() {
    var temperature = document.getElementById('Temperature');
    var humidity = document.getElementById('Humidity');
    
    var temperatureDocument = db.ref('SensorValues/Temperature');
    var humidityDocument = db.ref('SensorValues/Humidity');
    
    temperatureDocument.on('value', function(snapshot) {
        var data = snapshot.val();
        temperature.innerText = "Temperature: " + data;
    });
    humidityDocument.on('value', function(snapshot) {
        var data = snapshot.val();
        humidity.innerText = "Humidity: " + data;
    });
};

updateChart();