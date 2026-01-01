// Combined App.jsx
import React, { useState } from "react";
import { BrowserRouter, Routes, Route, useNavigate, useParams } from "react-router-dom";
import HomePage from "./pages/HomePage";
import BotsPage from "./pages/BotsPage";
import GamePage from "./pages/GamePage";
import LeaderboardPage from "./pages/LeaderboardPage";
import Header from "./components/Header";

// ... imports remain the same ...

export default function App() {
  const [user, setUser] = useState({ displayName: "Developer User" });
  const [directMatchId, setDirectMatchId] = useState(null);

  const handleExit = () => setDirectMatchId(null);

  return ( 
    <BrowserRouter> 
      <div className="app-container"> 
        <Header user={user} /> 
        <main> 
          {/* This logic check is what's causing the "Game Not Found" confusion */}
          {!directMatchId ? ( 
            <Routes> 
              <Route path="/" element={<HomePage />} /> 
              <Route path="/bots" element={<BotsPage setDirectMatchId={setDirectMatchId} />} /> 
              <Route path="/leaderboard" element={<LeaderboardPage />} />
              <Route path="/game/:matchId" element={<GamePageWrapper onExit={handleExit} />} /> 
            </Routes> 
          ) : ( 
            <GamePage matchId={directMatchId} onExit={handleExit} /> 
          )} 
        </main> 
      </div> 
    </BrowserRouter> 
  );
}

// Update the wrapper to receive and pass onExit
function GamePageWrapper({ onExit }) { 
  const { matchId } = useParams(); 
  const nav = useNavigate();

  // Create a custom exit that also cleans up the URL
  const handleWrapperExit = () => {
    onExit(); // Clear state
    nav("/"); // Go home
  };

  return <GamePage matchId={matchId} onExit={handleWrapperExit} />;
}
