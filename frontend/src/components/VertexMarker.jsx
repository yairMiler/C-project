import React from "react";

const PLAYER_COLORS = ["#e74c3c", "#3498db", "#f39c12", "#ecf0f1"];

export default function VertexMarker({ v, onClick }) {
  const isOwned = v.owner !== undefined && v.owner !== null && v.owner >= 0;
  const color = isOwned ? PLAYER_COLORS[v.owner] : "#fff";

  // SVG Path for a simple house (Settlement)
  const settlementPath = "M-7,3 L-7,-2 L0,-8 L7,-2 L7,3 Z";
  // SVG Path for a larger building/city (City)
  const cityPath = "M-9,5 L-9,-2 L-3,-2 L-3,-7 L3,-7 L3,-3 L9,-3 L9,5 Z";

  return (
    <g 
      transform={`translate(${v.x}, ${v.y})`} 
      onClick={onClick} 
      style={{ cursor: "pointer" }}
    >
      {!isOwned ? (
        // Interaction point for empty vertices
        <circle r={6} fill="white" fillOpacity={0.3} stroke="white" strokeWidth={1} />
      ) : (
        // Render the building shape
        <path
          d={v.type === "city" ? cityPath : settlementPath}
          fill={color}
          stroke="#333"
          strokeWidth={1.5}
          style={{ filter: 'drop-shadow(1px 2px 2px rgba(0,0,0,0.4))' }}
        />
      )}
    </g>
  );
}
