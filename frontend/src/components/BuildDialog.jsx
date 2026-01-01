// src/components/BuildDialog.jsx
import React from "react";

export default function BuildDialog({ open, vertexId, state, onClose, onBuild }) {
  if (!open) return null;
  const player = state.players[state.currentPlayer];

  return (
    <div className="dialog-overlay" style={{ background: "transparent", pointerEvents: "none", position: "absolute", width: "100%", height: "100%", top: 0, left: 0 }}>
      <div 
        className="dialog" 
        style={{ 
          pointerEvents: "auto",
          position: "absolute", 
          top: "50%",           
          right: "20px",        
          left: "auto",        
          transform: "translateY(-50%)", 
          width: "150px",      
          padding: "8px",      
          fontSize: "0.75rem", 
          zIndex: 1000
        }}
      >
        <h3 style={{ margin: "0 0 8px 0", fontSize: "0.9rem" }}>Vertex {vertexId}</h3>
        <div className="dialog-actions" style={{ display: "flex", flexDirection: "column", gap: "4px" }}>
          <button onClick={() => onBuild("settlement")}>Build Settlement</button>
          <button onClick={() => onBuild("city")}>Upgrade to City</button>
          <button onClick={onClose} style={{ marginTop: "4px" }}>Cancel</button>
        </div>
        <div className="hint" style={{ marginTop: "6px", fontSize: "0.65rem" }}>
          You: {player.userId || "player " + player.index}
        </div>
      </div>
    </div>
  );
}

