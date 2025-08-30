// Variables para controlar la actualización
let updateInterval;
const UPDATE_INTERVAL_MS = 2000; // Actualizar cada 2 segundos

// Función para formatear números con decimales
function formatNumber(number, decimals = 2) {
    return parseFloat(number).toFixed(decimals);
}

// Función para actualizar la interfaz con los datos recibidos
function updateUI(data) {
    try {
        // Actualizar datos del panel
        updateCard('panel', data.panel);
        
        // Actualizar datos de la batería
        updateCard('bateria', data.bateria);
        
        // Actualizar datos de la carga
        updateCard('carga', data.carga);
        
        console.log('Datos actualizados:', new Date().toLocaleTimeString());
    } catch (error) {
        console.error('Error al actualizar la interfaz:', error);
    }
}

// Función para actualizar una tarjeta específica
function updateCard(cardId, data) {
    // Actualizar LED de estado
    const ledElement = document.getElementById(`${cardId}-led`);
    if (ledElement) {
        ledElement.className = 'status-led';
        ledElement.classList.add(`led-${data.led}`);
    }
    
    // Actualizar valores numéricos
    updateValue(`${cardId}-voltaje`, formatNumber(data.voltaje));
    updateValue(`${cardId}-corriente`, formatNumber(data.corriente, 3));
    updateValue(`${cardId}-potencia`, formatNumber(data.potencia));
}

// Función para actualizar un valor en la interfaz con animación
function updateValue(elementId, value) {
    const element = document.getElementById(elementId);
    if (element) {
        // Solo actualizar si el valor cambió
        if (element.textContent !== value) {
            element.textContent = value;
            // Añadir clase de animación
            element.parentElement.classList.add('updated');
            // Quitar la clase después de la animación
            setTimeout(() => {
                element.parentElement.classList.remove('updated');
            }, 1000);
        }
    }
}

// Función para obtener datos del servidor
async function fetchData() {
    try {
        const response = await fetch('/data');
        if (!response.ok) {
            throw new Error(`Error HTTP: ${response.status}`);
        }
        const data = await response.json();
        updateUI(data);
    } catch (error) {
        console.error('Error al obtener datos:', error);
        // Reintentar después de un tiempo en caso de error
        setTimeout(fetchData, 5000);
    }
}

// Función para inicializar la aplicación
function init() {
    console.log('Iniciando aplicación de monitoreo solar...');
    
    // Cargar datos inmediatamente
    fetchData();
    
    // Configurar intervalo de actualización
    updateInterval = setInterval(fetchData, UPDATE_INTERVAL_MS);
    
    // Manejar la recarga de la página
    window.addEventListener('beforeunload', () => {
        clearInterval(updateInterval);
    });
}

// Iniciar la aplicación cuando el DOM esté listo
document.addEventListener('DOMContentLoaded', init);
