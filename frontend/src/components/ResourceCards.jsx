// src/components/ResourceCards.jsx
import React from "react";

function resourceName(id) {
  switch (Number(id)) {
    case 0: return "Brick";
    case 1: return "Lumber";
    case 2: return "Ore";
    case 3: return "Grain";
    case 4: return "Wool";
    default: return "Unknown";
  }
}

export default function ResourceCards({ resources }) {
  return (
    <div className="resource-cards">
      <h4>Resources</h4>
      <div className="cards-row">
        {Object.entries(resources).map(([k, v]) => (
          <div key={k} className="resource-card">
            <div className="rc-title">{resourceName(k)}</div>
            <div className="rc-count">{v}</div>
          </div>
        ))}
      </div>
    </div>
  );
}
