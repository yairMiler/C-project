import React from "react";
import { useNavigate } from "react-router-dom";

export default function HomePage() {
  const nav = useNavigate();

  
  const containerStyle = {
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
    justifyContent: "center",
    minHeight: "100vh", 
    gap: "25px",        
    backgroundColor: "#2c3e50", 
    color: "white"
  };

  const buttonStyle = {
    padding: "25px 60px",
    fontSize: "1.8rem",
    minWidth: "350px",
    cursor: "pointer",
    borderRadius: "12px",
    border: "none",
    backgroundColor: "#e67e22",
    color: "white",
    fontWeight: "bold",
    boxShadow: "0 4px 6px rgba(0,0,0,0.2)"
  };

  return (
    <div className="home" style={containerStyle}>
      <h1 style={{ fontSize: "5rem", marginBottom: "30px" }}>Catan</h1>
      <button style={buttonStyle} onClick={() => nav('/bots')}>
        Play vs Bots
      </button>
      <button style={buttonStyle}>
        Play Online
      </button>
      <button style={buttonStyle} onClick={() => nav('/leaderboard')}>
        Leaderboard
      </button>
    </div>
  );
}
