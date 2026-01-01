import React, { useState } from "react";
import { useNavigate } from "react-router-dom";
import { createMatch } from "../api/api";

export default function BotsPage() {
  const nav = useNavigate();
  const [playerName, setPlayerName] = useState("");
  const [showPrompt, setShowPrompt] = useState(false);
  const [selectedLevel, setSelectedLevel] = useState(null);

  const handleLevelSelect = (level) => {
    setSelectedLevel(level);
    setShowPrompt(true);
  };

  const start = async () => {
    if (!playerName.trim()) return alert("Please enter a name");

    const res = await createMatch({ 
      difficulty: selectedLevel, 
      playerName: playerName 
    }); 

    const id = res.id || res["id"] || res["matchId"]; 
    nav(`/game/${id}`); 
  }; 

  // Styles matching HomePage for consistency
  const containerStyle = {
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
    justifyContent: "center",
    minHeight: "100vh",
    gap: "25px",
    backgroundColor: "#2c3e50", // Dark background to match HomePage
    color: "white"
  };

  const buttonStyle = {
    padding: "25px 60px",
    fontSize: "1.8rem",
    minWidth: "350px",
    cursor: "pointer",
    borderRadius: "12px",
    border: "none",
    backgroundColor: "#e67e22", // Catan Orange
    color: "white",
    fontWeight: "bold",
    boxShadow: "0 4px 6px rgba(0,0,0,0.2)"
  };

  return ( 
    <div style={containerStyle}> 
      <h2 style={{ fontSize: "3rem", marginBottom: "20px" }}>Select Difficulty</h2> 
      <button style={buttonStyle} onClick={() => handleLevelSelect('easy')}>Easy</button> 
      <button style={buttonStyle} onClick={() => handleLevelSelect('medium')}>Medium</button> 
      <button style={buttonStyle} onClick={() => handleLevelSelect('hard')}>Hard</button> 

      { showPrompt && ( 
        <div style={{ 
          position: 'fixed', top: 0, left: 0, width: '100%', height: '100%', 
          backgroundColor: 'rgba(0,0,0,0.7)', display: 'flex', justifyContent: 'center', alignItems: 'center',
          zIndex: 1000
        }}> 
          <div style={{ background: 'white', padding: '20px', borderRadius: '8px', color: 'black', textAlign: 'center' }}> 
            <h3>Enter Your Name</h3> 
            <input 
              type="text" 
              value={playerName} 
              onChange={(e) => setPlayerName(e.target.value)} 
              placeholder="Your name..." 
              style={{ padding: '10px', width: '200px', marginBottom: '15px' }}
            /> 
            <div style={{ marginTop: '10px' }}> 
              <button onClick={start} style={{ marginRight: '10px', padding: '8px 15px' }}>Confirm & Start</button> 
              <button onClick={() => setShowPrompt(false)} style={{ padding: '8px 15px' }}>Cancel</button> 
            </div> 
          </div> 
        </div> 
      )} 
    </div> 
  );
}
