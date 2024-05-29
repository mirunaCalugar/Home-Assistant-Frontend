import React from "react";
import {
  GaugeContainer,
  GaugeValueArc,
  GaugeReferenceArc,
} from "@mui/x-charts/Gauge";
import GaugePointer from "./ArcDesign";

const getColorForTemperature = (temperature) => {
  if (temperature < 10) {
    return "blue"; // Rece
  } else if (temperature < 25) {
    return "green"; // Răcoros
  } else if (temperature < 35) {
    return "yellow"; // Cald
  } else {
    return "red"; // Fierbinte
  }
};

function TemperatureGauge({ temperature }) {
  const safeTemperature = Math.max(0, Math.min(temperature, 100));
  const gaugeColor = getColorForTemperature(safeTemperature);

  return (
    <GaugeContainer
      width={200}
      height={200}
      startAngle={-110}
      endAngle={110}
      value={safeTemperature}
    >
      <GaugeReferenceArc />
      <GaugeValueArc style={{ stroke: gaugeColor }} />
      <GaugePointer />
    </GaugeContainer>
  );
}

export default TemperatureGauge;
