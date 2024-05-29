from flask import Flask, jsonify, request, send_from_directory
import serial
import time
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from dotenv import load_dotenv
import os
from datetime import datetime

load_dotenv()

app = Flask(__name__, static_folder='../client/build', static_url_path='/')

# Configure serial port for communication with Arduino
try:
    ser = serial.Serial('COM3', 9600)  # Change serial port if necessary
    time.sleep(2)  # Wait for the serial connection to initialize
except Exception as e:
    ser = None
    print(f"Failed to connect to serial port: {e}")

# In-memory storage for the last 10 messages and events
messages = []
events = []

EVENT_START_ADDR = 0
EVENT_MAX_LENGTH = 32
MAX_EVENTS = 10

# Function to read events from in-memory storage (or from EEPROM in the future)
def read_stored_events():
    return events

# Function to store an event in in-memory storage (or in EEPROM in the future)
def store_event(event):
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    if len(events) >= MAX_EVENTS:
        events.pop(0)  # Remove the oldest event if we have more than MAX_EVENTS
    events.append({'timestamp': timestamp, 'event': event})

# Function to read data from the serial port
def read_serial_data(command):
    try:
        ser.write(command)
        data = ser.readline().decode().strip()
        print(f"Raw data received from Arduino: '{data}'")
        return data
    except Exception as e:
        print(f"Error reading from serial port: {e}")
        return None

# Function to send email notification
def send_email_notification():
    msg = MIMEMultipart()
    msg['From'] = app.config['MAIL_USERNAME']
    msg['To'] = app.config['MAIL_RECEIVER']
    msg['Subject'] = "Flood Alert!"
    body = "The water level has exceeded the threshold. Please take necessary actions."
    msg.attach(MIMEText(body, 'plain'))

    try:
        with smtplib.SMTP(app.config['MAIL_SERVER'], app.config['MAIL_PORT']) as server:
            server.starttls()  # Upgrade the connection to TLS
            server.login(app.config['MAIL_USERNAME'], app.config['MAIL_PASSWORD'])
            server.sendmail(app.config['MAIL_USERNAME'], app.config['MAIL_RECEIVER'], msg.as_string())
        print("Email sent successfully!")
    except Exception as e:
        print(f"Failed to send email: {e}")

@app.route('/')
def index():
    return send_from_directory(app.static_folder, 'index.html')

@app.route('/sensors', methods=['GET'])
def sensors():
    if ser is None:
        return jsonify({'error': 'Serial port not initialized'}), 500

    print("Fetching sensor data...")
    data = read_serial_data(b'R')
    if data:
        try:
            parts = data.split(',')
            temperature, humidity, water_level = map(float, parts)
            print(f"Sensor data fetched: Temperature={temperature}, Humidity={humidity}, Water Level={water_level}")

            if water_level > 30:
                send_email_notification()
                store_event("Flood detected")

            return jsonify({
                'temperature': temperature,
                'humidity': humidity,
                'waterLevel': water_level
            })
        except ValueError as e:
            print(f"Invalid sensor data received: {data} ({e})")
            return jsonify({'error': f'Invalid sensor data received: {data}'}), 500
    else:
        print("Error reading sensor data")
        return jsonify({'error': 'Error reading sensor data'}), 500

@app.route('/control/<action>', methods=['POST'])
def control(action):
    if ser is None:
        return jsonify({'error': 'Serial port not initialized'}), 500

    if action == 'on':
        ser.write(b'A')
    elif action == 'off':
        ser.write(b'S')
    else:
        return jsonify({'error': 'Invalid action'}), 400
    return jsonify({'status': 'success', 'action': action})

@app.route('/send-message', methods=['POST'])
def send_message():
    message = request.json.get('message')
    if message:
        if len(messages) >= 10:
            messages.pop(0)
        messages.append(message)
        
        ser.write(f'U{message}\n'.encode())
        return jsonify({'status': 'success', 'message': message})
    else:
        return jsonify({'status': 'error', 'message': 'No message provided'}), 400

@app.route('/messages', methods=['GET'])
def get_messages():
    return jsonify({'messages': messages})

@app.route('/events', methods=['GET'])
def events_endpoint():
    return jsonify({'events': read_stored_events()})


@app.route('/delete-event', methods=['DELETE'])
def delete_event():
    index = request.args.get('index')
    if index is not None and index.isdigit():
        index = int(index)
        if 0 <= index < len(events):
            events.pop(index)
            return jsonify({'status': 'success', 'index': index})
    return jsonify({'status': 'error', 'message': 'Invalid event index'}), 400


if __name__ == '__main__':
    app.config['MAIL_SERVER'] = os.getenv('MAIL_SERVER')
    app.config['MAIL_PORT'] = int(os.getenv('MAIL_PORT'))
    app.config['MAIL_USERNAME'] = os.getenv('MAIL_USERNAME')
    app.config['MAIL_PASSWORD'] = os.getenv('MAIL_PASSWORD')
    app.config['MAIL_USE_TLS'] = os.getenv('MAIL_USE_TLS', 'True').lower() == 'true'
    app.config['MAIL_USE_SSL'] = os.getenv('MAIL_USE_SSL', 'False').lower() == 'true'
    app.config['MAIL_RECEIVER'] = os.getenv('MAIL_RECEIVER')
    app.run(host='127.0.0.1', port=5500)
