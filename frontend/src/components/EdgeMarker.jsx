import React from "react";

// Standard Catan Colors: Red, Blue, Orange, White
const PLAYER_COLORS = ["#e74c3c", "#3498db", "#f39c12", "#ecf0f1"];

export default function EdgeMarker({ x1, y1, x2, y2, edge, onClick }) {
  const isOwned = edge.owner !== undefined && edge.owner !== null && edge.owner >= 0;
  const stroke = isOwned ? PLAYER_COLORS[edge.owner] : "rgba(0,0,0,0.1)";
  
  return (
    <g onClick={onClick} style={{ cursor: "pointer" }}>
      {/* Invisible wider line for easier clicking */}
      <line x1={x1} y1={y1} x2={x2} y2={y2} stroke="transparent" strokeWidth={15} />
      
      {/* The actual road */}
      <line 
        x1={x1} y1={y1} x2={x2} y2={y2} 
        stroke={stroke} 
        strokeWidth={isOwned ? 7 : 2} 
        strokeLinecap="round"
        style={{ 
          transition: 'all 0.3s ease',
          filter: isOwned ? 'drop-shadow(0px 0px 2px rgba(0,0,0,0.5))' : 'none'
        }}
      />
    </g>
  );
}
