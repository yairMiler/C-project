// src/pages/GamePage.jsx
import React, { useEffect, useState } from "react";
import { getState, doAction } from "../api/api";
import Board from "../components/Board";
import Toolbar from "../components/Toolbar";
import PlayerPanel from "../components/PlayerPanel";
import GameClock from "../components/GameClock";

const PLAYER_COLORS = ["#e74c3c", "#3498db", "#f39c12", "#ecf0f1"];

export default function GamePage({ matchId, onExit }) { 
    const [state, setState] = useState(null); 
    const [loading, setLoading] = useState(true);
    const [processingTimeout, setProcessingTimeout] = useState(false); 

    async function refresh() { 
        try { 
            const s = await getState(matchId); 
            console.log("Data received from getState:", s); 
            setState(s); 
            // Reset timeout processing if we successfully got a new state
            setProcessingTimeout(false);
        } catch (e) { 
            console.error(e); 
        } finally { 
            setLoading(false); 
        } 
    } 

    // Polling Effect
    useEffect(() => { 
        refresh(); 
        const t = setInterval(refresh, 2500); 
        return () => clearInterval(t); 
    }, [matchId]); 

    // Timeout Monitoring Effect
    useEffect(() => {
        const timer = setInterval(async () => {
            if (state && state.turnDeadline && !processingTimeout && state.winner === -1) {
                const now = Math.floor(Date.now() / 1000);
                if (now >= state.turnDeadline) {
                    setProcessingTimeout(true);
                    console.log("Deadline reached! Sending auto_play action...");
                    await doAct({ action: "auto_play" });
                }
            }
        }, 1000);

        return () => clearInterval(timer);
    }, [state, processingTimeout]);

    async function doAct(actionBody) { 
        try {
            const res = await doAction(matchId, actionBody); 
            setState(res); 
            return res; 
        } catch (e) {
            console.error("Action failed:", e);
            setProcessingTimeout(false);
        }
    } 

    if (loading) return <div className="loading">Loading game...</div>; 
    if (!state) return <div>Game not found</div>; 

    if (state && state.winner !== -1) {
        const winner = state.players[state.winner];
        return (
            <div className="victory-screen" style={{
                position: 'fixed', top: 0, left: 0, width: '100vw', height: '100vh',
                backgroundColor: '#1a1a2e', display: 'flex', flexDirection: 'column',
                justifyContent: 'center', alignItems: 'center', zIndex: 2000, color: 'gold'
            }}>
                <h1 style={{ fontSize: '5rem', marginBottom: '0' }}>🎊 VICTORY! 🎊</h1>
                <div style={{ fontSize: '8rem' }}>🏆</div>
                <h2 style={{ fontSize: '3rem' }}>{winner.name}</h2>
                <p style={{ fontSize: '1.5rem', color: 'white' }}>Final Score: {winner.score} Victory Points</p>
                <button 
                    onClick={onExit}
                    style={{
                        marginTop: '30px', padding: '15px 40px', fontSize: '1.2rem',
                        backgroundColor: 'gold', border: 'none', borderRadius: '5px', cursor: 'pointer', fontWeight: 'bold'
                    }}
                >
                    Back to Main Menu
                </button>
                <div style={{ marginTop: '20px', fontSize: '2rem' }}>✨🏰✨</div>
            </div>
        );
    }

    const turnIdx = state.currentPlayer ?? 0;
    const turnColor = PLAYER_COLORS[turnIdx] || "#34495e";

    return ( 
        <div className="game-screen" style={{ 
          backgroundColor: "#2c3e50", 
          height: "100vh", 
          display: 'flex', 
          flexDirection: 'row', 
          overflow: 'hidden'
        }}> 
            {/* Left column for Board */} 
            <div className="left-col" style={{ 
              flex: '1', 
              backgroundColor: "#e67e22", 
              borderRadius: "8px", 
              margin: "10px", 
              position: 'relative'
            }}>
                <Board state={state} onAction={doAct} /> 
            </div> 

            {/* Right column for Panels, Clock, and Turn Indicator */} 
            <div className="right-col" style={{ 
              width: '320px', 
              display: 'flex', 
              flexDirection: 'column', 
              padding: '10px', 
              gap: '10px',
              zIndex: 10 
            }}> 
                <PlayerPanel state={state} onAction={doAct} /> 
                <Toolbar state={state} onAction={doAct} /> 
                
                { state && state.turnDeadline && ( 
                    <GameClock deadline={state.turnDeadline} /> 
                )}

                {/* Turn Indicator Block - Matching Clock Size */}
                <div className="turn-indicator" style={{
                    padding: "15px",
                    backgroundColor: turnColor,
                    color: turnIdx === 3 ? "black" : "white",
                    borderRadius: "8px",
                    fontWeight: "bold",
                    textAlign: "center",
                    fontSize: "1.2rem",
                    border: "1px solid rgba(0,0,0,0.1)",
                    boxShadow: "0 2px 4px rgba(0,0,0,0.2)",
                    textTransform: "uppercase",
                    opacity: processingTimeout ? 0.7 : 1 // Dim slightly when bot is moving
                }}>
                    {processingTimeout ? "Auto-playing..." : `Turn: Player ${turnIdx}`}
                </div>
                
                <div className="topbar" style={{ marginTop: 'auto', color: 'white' }}> 
                    <button onClick={onExit}>Exit Game</button> 
                    <div style={{ fontSize: '0.8rem', marginTop: '5px' }}>Match: {matchId}</div> 
                </div> 
            </div> 
        </div> 
    );
}

