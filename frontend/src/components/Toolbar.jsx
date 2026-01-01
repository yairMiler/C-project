// src/components/Toolbar.jsx
import React, { useState } from "react";

export default function Toolbar({ state, onAction }) {
  const [trade, setTrade] = useState({ give: "0", amount: 4, receive: "0" });

  const inputStyle = { backgroundColor: "transparent", color: "white", border: "1px solid white", borderRadius: "4px", padding: "2px", fontSize: "0.8rem" };

  async function roll() { await onAction({ action: "roll" }); }
  async function endTurn() { await onAction({ action: "end_turn" }); }
  async function buyDev() { await onAction({ action: "buy_dev" }); }
  async function tradeWithBank() { await onAction({ action: "trade_with_bank", give: trade.give, amount: trade.amount, receive: trade.receive, rate: trade.amount }); }

  return (
    <div className="toolbar" style={{ backgroundColor: "#e67e22", color: "white", padding: "10px", borderRadius: "8px" }}>
      <h4 style={{ color: "white", margin: "0 0 5px 0", fontSize: "1rem" }}>Actions</h4>
      <div style={{ marginBottom: "8px" }}>
        <button onClick={roll} style={{ padding: "2px 5px", fontSize: "0.8rem" }}>Roll</button>
        <button onClick={buyDev} style={{ padding: "2px 5px", fontSize: "0.8rem", margin: "0 4px" }}>Buy Dev</button>
        <button onClick={endTurn} style={{ padding: "2px 5px", fontSize: "0.8rem" }}>End Turn</button>
      </div>
      <div className="trade-panel" style={{ borderTop: "1px solid rgba(255,255,255,0.2)", paddingTop: "5px" }}>
        <h5 style={{ color: "white", margin: "0 0 5px 0", fontSize: "0.9rem" }}>Bank Trade</h5>
        <div style={{ display: "flex", flexWrap: "wrap", gap: "5px", alignItems: "center", fontSize: "0.8rem" }}>
          <label>Give</label>
          <select style={inputStyle} value={trade.give} onChange={(e) => setTrade({ ...trade, give: e.target.value })}>
            <option value="0" style={{color: "black"}}>Brick</option>
            <option value="1" style={{color: "black"}}>Lumber</option>
            <option value="2" style={{color: "black"}}>Ore</option>
            <option value="3" style={{color: "black"}}>Grain</option>
            <option value="4" style={{color: "black"}}>Wool</option>
          </select>
          <label>Amt</label>
          <input style={{...inputStyle, width: "35px"}} type="number" value={trade.amount} onChange={(e) => setTrade({ ...trade, amount: Number(e.target.value) })} />
          <label>Get</label>
          <select style={inputStyle} value={trade.receive} onChange={(e) => setTrade({ ...trade, receive: e.target.value })}>
            <option value="0" style={{color: "black"}}>Brick</option>
            <option value="1" style={{color: "black"}}>Lumber</option>
            <option value="2" style={{color: "black"}}>Ore</option>
            <option value="3" style={{color: "black"}}>Grain</option>
            <option value="4" style={{color: "black"}}>Wool</option>
          </select>
          <button onClick={tradeWithBank} style={{ padding: "2px 5px", fontSize: "0.8rem" }}>Trade</button>
        </div>
      </div>
    </div>
  );
}
