// src/components/Dice.jsx
import React, { useState } from "react";

export default function Dice({ onRoll }) {
  const [rolling, setRolling] = useState(false);
  const [last, setLast] = useState(null);

  async function roll() {
    setRolling(true);
    // quick CSS animated spinner, then request
    const res = await onRoll();
    setLast(res.dice || res.roll || res.currentDice || null);
    setTimeout(() => setRolling(false), 700);
  }

  return (
    <div className="dice-widget">
      <div className={`dice ${rolling ? "rolling" : ""}`} onClick={roll}>
        {rolling ? "🎲" : (last || "🎲")}
      </div>
      <div><small>Click to roll</small></div>
    </div>
  );
}
