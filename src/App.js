import React, { useState, useEffect } from "react";
import "./App.css";
import Navbar from "./NavBar";
import smartHomeIcon from "./undraw_smart_home_re_orvn.svg";
import electricityIcon from "./undraw_electricity_k2ft.svg";
import thermometerIcon from "./thermometer-svgrepo-com.svg";
import humidityIcon from "./humidity-svgrepo-com.svg";
import waterLevelIcon from "./water-level-svgrepo-com.svg";
import { Gauge } from "@mui/x-charts/Gauge";

function App() {
  const [temperature, setTemperature] = useState(null);
  const [humidity, setHumidity] = useState(null);
  const [waterLevel, setWaterLevel] = useState(null);
  const [message, setMessage] = useState("");
  const [messages, setMessages] = useState([]);
  const [events, setEvents] = useState([]);
  const [ledState, setLedState] = useState(false);
  const [error, setError] = useState(null);

  // Fetch sensor data from the backend
  const fetchSensorData = async () => {
    try {
      const response = await fetch("/sensors");
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      const data = await response.json();
      setTemperature(parseFloat(data.temperature));
      setHumidity(parseFloat(data.humidity));
      setWaterLevel(parseFloat(data.waterLevel));
    } catch (error) {
      console.error("Error fetching sensor data:", error);
      setError("Error fetching sensor data. Please try again later.");
    }
  };
  const fetchEvents = async () => {
    try {
      const response = await fetch("/events");
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      const data = await response.json();
      setTimeout(() => {
        console.log("DATA: ", data);
      }, 3000);
      // Verifică datele primite de la backend
      if (data.events) {
        setEvents(data.events);
      }
    } catch (error) {
      console.error("Error fetching events:", error);
      setError("Error fetching events");
    }
  };

  // Fetch messages from the backend
  const fetchMessages = async () => {
    try {
      const response = await fetch("/messages");
      const data = await response.json();
      setMessages(data.messages);
    } catch (error) {
      console.error("Error fetching messages:", error);
    }
  };

  const sendMessage = async () => {
    try {
      const response = await fetch("/send-message", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ message }),
      });
      const data = await response.json();
      if (data.status === "success") {
        setMessages((prevMessages) => {
          const newMessages = [...prevMessages, message];
          if (newMessages.length > 10) {
            newMessages.shift(); // Remove the oldest message if we have more than 10
          }
          return newMessages;
        });
        setMessage("");
      } else {
        console.error("Error sending message:", data);
      }
    } catch (error) {
      console.error("Error sending message:", error);
    }
  };

  // Toggle LED on/off
  const toggleLed = async (action) => {
    try {
      const response = await fetch(`/control/${action}`, {
        method: "POST",
      });
      const data = await response.json();
      if (data.status === "success") {
        setLedState(action === "on");
      } else {
        console.error(`Error toggling LED ${action}:`, data);
      }
    } catch (error) {
      console.error(`Error toggling LED ${action}:`, error);
    }
  };

  const handleCheckboxChange = (event) => {
    const action = event.target.checked ? "on" : "off";
    toggleLed(action);
  };

  // Delete an event from the backend
  // Delete an event from the backend
  const deleteEvent = async (index) => {
    try {
      const response = await fetch(`/delete-event?index=${index}`, {
        method: "DELETE",
      });
      const data = await response.json();
      if (data.status === "success") {
        // Elimină evenimentul din lista de evenimente afișate
        setEvents((prevEvents) => prevEvents.filter((_, i) => i !== index));
      } else {
        console.error("Error deleting event:", data);
        setError("Error deleting event");
      }
    } catch (error) {
      console.error("Error deleting event:", error);
      setError("Error deleting event");
    }
  };

  useEffect(() => {
    fetchSensorData();
    fetchMessages();
    fetchEvents();

    // Optionally, fetch sensor data periodically
    const intervalId = setInterval(fetchSensorData, 5000); // every 5 seconds
    return () => clearInterval(intervalId);
  }, []);

  const formatDate = (timestamp) => {
    const date = new Date(timestamp);
    return date.toLocaleString();
  };

  return (
    <div className="App">
      <div className="App-header">
        <Navbar />
        <div className="func-container">
          <div className="div1">
            <img
              src={electricityIcon}
              alt="Electricity Icon"
              className="electricity-icon"
            />
            <label className="switch">
              <input
                type="checkbox"
                checked={ledState}
                onChange={handleCheckboxChange}
              />
              <span className="slider"></span>
            </label>
          </div>
          <div className="div1">
            <p className="p">
              Temperatura: {temperature !== null ? `${temperature}°C` : "N/A"}
            </p>
            <img
              src={thermometerIcon}
              alt="Thermometer Icon"
              className="thermometer-icon"
            />
            {temperature !== null && (
              <Gauge
                width={150}
                height={150}
                value={temperature}
                startAngle={0}
                endAngle={360}
              />
            )}
          </div>
          <div className="div1">
            <p className="p">
              Umiditatea: {humidity !== null ? `${humidity}% ` : "N/A"}
            </p>
            <img
              src={humidityIcon}
              alt="Humidity Icon"
              className="thermometer-icon"
            />
            {humidity !== null && (
              <Gauge
                width={150}
                height={150}
                value={humidity}
                startAngle={0}
                endAngle={360}
              />
            )}
          </div>
          <div className="div1">
            <p className="p">
              Nivelul apei: {waterLevel !== null ? `${waterLevel}% ` : "N/A"}
            </p>
            <img
              src={waterLevelIcon}
              alt="Water Level Icon"
              className="thermometer-icon"
            />
            {waterLevel !== null && (
              <Gauge
                width={150}
                height={150}
                value={waterLevel}
                startAngle={0}
                endAngle={360}
              />
            )}
          </div>
        </div>
        <div className="div2">
          <div className="divmessage">
            <div className="form">
              <input
                placeholder="Type your text"
                className="input"
                type="text"
                value={message}
                onChange={(e) => setMessage(e.target.value)}
              />
              <span className="input-border"></span>
            </div>
            <div>
              <button onClick={sendMessage}>
                <span className="circle1"></span>
                <span className="circle2"></span>
                <span className="circle3"></span>
                <span className="circle4"></span>
                <span className="circle5"></span>
                <span className="text">Send</span>
              </button>
            </div>
          </div>
          <div className="message-container">
            <p>Messages</p>
            <ul>
              {messages.map((msg, index) => (
                <li key={index}>{msg}</li>
              ))}
            </ul>
          </div>
        </div>
        <div className="div2">
          <div className="events">
            <h2>Flood Events</h2>
            <ul>
              {events
                .filter((event) => event.event === "Flood detected") // Filtrare pentru evenimente de inundații
                .map((event, index) => (
                  <li>
                    {formatDate(event.timestamp)} - {event.event}
                    <button onClick={() => deleteEvent(index)}>Delete</button>
                  </li>
                ))}
            </ul>
          </div>
          {error && <div className="error">{error}</div>}
        </div>
      </div>
      <img
        src={smartHomeIcon}
        alt="Smart Home Icon"
        className="smart-home-icon"
      />
    </div>
  );
}

export default App;
