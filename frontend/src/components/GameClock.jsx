import React, { useState, useEffect } from "react";

export default function GameClock({ deadline }) {
    const [secondsLeft, setSecondsLeft] = useState(0);

    useEffect(() => {
        const updateTimer = () => {
            const now = Math.floor(Date.now() / 1000);
            const diff = deadline - now;
            setSecondsLeft(diff > 0 ? diff : 0);
        };
        updateTimer();
        const interval = setInterval(updateTimer, 1000);
        return () => clearInterval(interval);
    }, [deadline]);

    const mins = Math.floor(secondsLeft / 60);
    const secs = (secondsLeft % 60).toString().padStart(2, '0');

    return (
        <div className="game-clock" style={{
            padding: "15px",
            backgroundColor: "#e67e22",
            color: "white",
            borderRadius: "8px",
            fontWeight: "bold",
            textAlign: "center",
            fontSize: "1.2rem",
            border: "1px solid rgba(255,255,255,0.2)"
        }}>
            Time: {mins}:{secs}
        </div>
    );
}