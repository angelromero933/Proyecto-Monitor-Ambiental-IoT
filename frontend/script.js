document.addEventListener("DOMContentLoaded", function () {
    // Ajustes estéticos de Chart.js
    Chart.defaults.color = '#ffffff';

    const canvasTiempoReal = document.getElementById("graficaTiempoReal");
    const canvasDona = document.getElementById("graficaDona");
    const selectorTipo = document.getElementById("tipoGrafico");

    if (!canvasTiempoReal || !canvasDona) {
        console.error("No se encontraron los elementos canvas en el DOM.");
        return;
    }

    // 1. Gráfica Tiempo Real (Línea)
    const graficaTiempoReal = new Chart(canvasTiempoReal, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                {
                    label: 'Temperatura (°C)',
                    data: [],
                    borderColor: '#ff6384',
                    backgroundColor: 'rgba(255, 99, 132, 0.2)',
                    borderWidth: 2,
                    fill: false
                },
                {
                    label: 'Humedad (%)',
                    data: [],
                    borderColor: '#36a2eb',
                    backgroundColor: 'rgba(54, 162, 235, 0.2)',
                    borderWidth: 2,
                    fill: false
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: { beginAtZero: true }
            }
        }
    });

    // 2. Gráfica de Dona
    const graficaDona = new Chart(canvasDona, {
        type: 'doughnut',
        data: {
            labels: ['Temperatura (°C)', 'Humedad (%)'],
            datasets: [{
                data: [22, 45],
                backgroundColor: ['#ff6384', '#36a2eb']
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false
        }
    });

    // 3. Función simuladora
    function actualizarSimulacion() {
        const horaActual = new Date().toLocaleTimeString();
        const temp = Math.floor(Math.random() * (28 - 18 + 1)) + 18;
        const humedad = Math.floor(Math.random() * (60 - 30 + 1)) + 30;

        // Actualizar línea
        graficaTiempoReal.data.labels.push(horaActual);
        graficaTiempoReal.data.datasets[0].data.push(temp);
        graficaTiempoReal.data.datasets[1].data.push(humedad);

        if (graficaTiempoReal.data.labels.length > 8) {
            graficaTiempoReal.data.labels.shift();
            graficaTiempoReal.data.datasets[0].data.shift();
            graficaTiempoReal.data.datasets[1].data.shift();
        }
        graficaTiempoReal.update();

        // Actualizar dona
        graficaDona.data.datasets[0].data = [temp, humedad];
        graficaDona.update();
    }

    // Primera ejecución inmediata
    actualizarSimulacion();

    // Repetir cada 3 segundos
    setInterval(actualizarSimulacion, 3000);

    // Cambiar tipo de gráfica con el selector
    if (selectorTipo) {
        selectorTipo.addEventListener("change", function (e) {
            graficaTiempoReal.config.type = e.target.value;
            graficaTiempoReal.update();
        });
    }
});


//ldeabajo
window.addEventListener("load", function () {
    // Verificar si Chart.js se descargó correctamente
    if (typeof Chart === 'undefined') {
        alert("Error: No se pudo cargar la librería Chart.js. Verifica tu conexión a internet.");
        return;
    }

    Chart.defaults.color = '#ffffff';

    const canvasTiempoReal = document.getElementById("graficaTiempoReal");
    const canvasDona = document.getElementById("graficaDona");
    const selectorTipo = document.getElementById("tipoGrafico");

    // 1. Gráfica Tiempo Real
    const graficaTiempoReal = new Chart(canvasTiempoReal, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                {
                    label: 'Temperatura (°C)',
                    data: [],
                    borderColor: '#ff6384',
                    backgroundColor: 'rgba(255, 99, 132, 0.2)',
                    borderWidth: 2
                },
                {
                    label: 'Humedad (%)',
                    data: [],
                    borderColor: '#36a2eb',
                    backgroundColor: 'rgba(54, 162, 235, 0.2)',
                    borderWidth: 2
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: { beginAtZero: true }
            }
        }
    });

    // 2. Gráfica de Dona
    const graficaDona = new Chart(canvasDona, {
        type: 'doughnut',
        data: {
            labels: ['Temperatura (°C)', 'Humedad (%)'],
            datasets: [{
                data: [22, 45],
                backgroundColor: ['#ff6384', '#36a2eb']
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false
        }
    });

    // 3. Simulación de sensores
    function actualizarSimulacion() {
        const horaActual = new Date().toLocaleTimeString();
        const temp = Math.floor(Math.random() * (28 - 18 + 1)) + 18;
        const humedad = Math.floor(Math.random() * (60 - 30 + 1)) + 30;

        graficaTiempoReal.data.labels.push(horaActual);
        graficaTiempoReal.data.datasets[0].data.push(temp);
        graficaTiempoReal.data.datasets[1].data.push(humedad);

        if (graficaTiempoReal.data.labels.length > 8) {
            graficaTiempoReal.data.labels.shift();
            graficaTiempoReal.data.datasets[0].data.shift();
            graficaTiempoReal.data.datasets[1].data.shift();
        }
        graficaTiempoReal.update();

        graficaDona.data.datasets[0].data = [temp, humedad];
        graficaDona.update();
    }

    actualizarSimulacion();
    setInterval(actualizarSimulacion, 3000);

    if (selectorTipo) {
        selectorTipo.addEventListener("change", function (e) {
            graficaTiempoReal.config.type = e.target.value;
            graficaTiempoReal.update();
        });
    }
});