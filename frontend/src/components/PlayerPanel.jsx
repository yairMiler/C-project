// src/components/PlayerPanel.jsx
import React from "react";

const RESOURCE_COLORS = {
  0: "#c27f3b", // Brick
  1: "#2e8b57", // Lumber
  2: "#9b8f8f", // Ore
  3: "#f5e79e", // Grain
  4: "#b2d08e", // Wool
  5: "#e8d7b5"
};

const RESOURCE_NAMES = ["Brick", "Lumber", "Ore", "Grain", "Wool"];

export default function PlayerPanel({ state, onAction }) {
  const me = state.players[state.currentPlayer];

  return (
    <div className="player-panel" style={{ 
      height: 'auto', 
      backgroundColor: "#e67e22", 
      color: "white", 
      padding: "10px", 
      borderRadius: "8px", 
      marginBottom: "10px" 
    }}>
      <h3 style={{ margin: '0 0 5px 0', color: "white", fontSize: "1.1rem" }}>Player {me.index} — {me.userId}</h3>
      <div style={{ fontSize: "0.9rem" }}>Victory: {me.victoryPoints} | Knights: {me.knightsPlayed}</div>
      <div style={{ fontSize: "0.9rem" }}>Longest Road: {me.hasLongestRoad ? "Yes" : "No"} | Largest Army: {me.hasLargestArmy ? "Yes" : "No"}</div>

      {/* Resources Section */}
      <div className="resources-section" style={{ marginTop: '8px', minHeight: '60px' }}>
        <h4 style={{ margin: '0 0 5px 0', color: "white", fontSize: "0.9rem" }}>Resources</h4>
        <div style={{ display: 'flex', gap: '3px', flexWrap: 'nowrap' }}>
          {RESOURCE_NAMES.map((name, index) => (
            <div key={index} style={{
              backgroundColor: RESOURCE_COLORS[index],
              color: index === 3 ? 'black' : 'white',
              padding: '4px 2px',
              borderRadius: '4px',
              fontSize: '0.65rem',
              width: '48px',
              textAlign: 'center',
              border: '1px solid rgba(0,0,0,0.2)',
              flexShrink: 0
            }}>
              <div style={{ fontWeight: 'bold' }}>{name}</div>
              <div style={{ fontSize: '0.9rem' }}>{me.resources[index] || 0}</div>
            </div>
          ))}
        </div>
      </div>

      {/* Dev Cards Section - Enlarged to match resource section space */}
      <div className="devcards" style={{ marginTop: '8px', minHeight: '60px' }}>
        <h4 style={{ margin: '0', color: "white", fontSize: "0.9rem" }}>Dev Cards</h4>
        <ul style={{ paddingLeft: '20px', margin: 0 }}>
          {me.devCards.map((c, i) => <li key={i} style={{ fontSize: '0.85rem' }}>{c}</li>)}
        </ul>
      </div>
    </div>
  );
}