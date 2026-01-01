// src/components/BanditModal.jsx
import React from "react";

export default function BanditModal({ open, state, onPick, onClose }) {
  if (!open) return null;

  return (
    <div className="dialog-overlay" style={{
      position: 'fixed', top: 0, left: 0, right: 0, bottom: 0,
      background: 'rgba(0,0,0,0.8)', display: 'flex', 
      justifyContent: 'center', alignItems: 'center', zIndex: 1000
    }}>
      <div className="dialog" style={{ 
        background: '#2c3e50', padding: '30px', borderRadius: '12px', 
        border: '2px solid #e67e22', textAlign: 'center', color: 'white'
      }}>
        <h2 style={{ color: '#e67e22' }}>🥷 Move the Bandit</h2>
        <p>A 7 was rolled! Choose a new hex for the robber.</p>
        
        <div style={{ 
          display: 'grid', gridTemplateColumns: 'repeat(4, 1fr)', 
          gap: '10px', margin: '20px 0', maxHeight: '300px', overflowY: 'auto' 
        }}>
          {state.hexes.map((hex) => (
            <button 
              key={hex.id} 
              disabled={hex.id === state.robber}
              onClick={() => onPick(hex.id)}
              style={{
                padding: '10px',
                borderRadius: '8px',
                border: 'none',
                background: hex.id === state.robber ? '#7f8c8d' : '#d35400',
                color: 'white',
                cursor: hex.id === state.robber ? 'not-allowed' : 'pointer',
                fontWeight: 'bold'
              }}
            >
              Hex {hex.id} <br/>
              <span style={{ fontSize: '0.8rem', opacity: 0.8 }}>({hex.resource})</span>
            </button>
          ))}
        </div>
        
        <button 
          onClick={onClose}
          style={{ 
            background: 'transparent', border: '1px solid #7f8c8d', 
            color: '#7f8c8d', padding: '8px 16px', borderRadius: '5px', cursor: 'pointer'
          }}
        >
          Close
        </button>
      </div>
    </div>
  );
}