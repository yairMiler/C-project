import React, { useMemo, useState, useEffect } from "react";
import Tile, { hexPoints } from "./Tile";
import EdgeMarker from "./EdgeMarker";
import VertexMarker from "./VertexMarker";
import BuildDialog from "./BuildDialog";
import BanditModal from "./BanditModal";

const HEX_SIZE = 68; 
const HEX_W = Math.sqrt(3) * HEX_SIZE;
const HEX_H = 1.5 * HEX_SIZE;
const ROWS = [3, 4, 5, 4, 3];
const BOARD_CENTER = { x: 500, y: 370 };

const PORT_DATA = {
  1: { label: "3:1", icon: "⚓" },
  2: { label: "2:1", icon: "🧱" },
  3: { label: "2:1", icon: "🌲" },
  4: { label: "2:1", icon: "🌾" },
  5: { label: "2:1", icon: "🐑" },
  6: { label: "2:1", icon: "🏔️" },
};

export default function Board({ state, onAction }) {
  const [buildOpen, setBuildOpen] = useState(false);
  const [selectedVertex, setSelectedVertex] = useState(null);
  const [banditOpen, setBanditOpen] = useState(false);
  const [isFlashing, setIsFlashing] = useState(false);

  // Trigger Flash and Bandit Modal on a 7
  useEffect(() => {
    if (state.dice > 0) {
      setIsFlashing(true);
      const timer = setTimeout(() => setIsFlashing(false), 600);
      
      // AUTO-OPEN Bandit Modal if 7 is rolled
      if (state.dice === 7) {
        setBanditOpen(true);
      }
      
      return () => clearTimeout(timer);
    }
  }, [state.dice]);

  const tileCenters = useMemo(() => {
    let idx = 0;
    const res = new Map();
    const startX = 500;
    const startY = 120; 
    ROWS.forEach((count, row) => {
      const y = startY + row * HEX_H;
      const rowWidth = (count - 1) * HEX_W;
      for (let i = 0; i < count; i++) {
        const x = startX - rowWidth / 2 + i * HEX_W;
        const hex = state.hexes[idx++];
        if (hex) res.set(hex.id, { x, y });
      }
    });
    return res;
  }, [state.hexes]);

  const vertexPos = useMemo(() => {
    const map = new Map();
    state.hexes.forEach(hex => {
      if (!Array.isArray(hex.adjacentVertices)) return;
      const center = tileCenters.get(hex.id);
      if (!center) return;
      const pts = hexPoints(center.x, center.y, HEX_SIZE);
      hex.adjacentVertices.forEach((vid, cornerIdx) => {
        if (vid != null && !map.has(vid)) map.set(vid, pts[cornerIdx]);
      });
    });
    return map;
  }, [state.hexes, tileCenters]);

  const getDiceFaces = (total) => {
    if (!total || total < 2) return ["🎲", "🎲"];
    const die1 = Math.floor(Math.random() * (total - 1)) + 1;
    const d1 = die1 > 6 ? 6 : die1;
    const d2 = total - d1;
    const icons = ["", "⚀", "⚁", "⚂", "⚃", "⚄", "⚅"];
    return [icons[d1], icons[d2]];
  };

  const [face1, face2] = getDiceFaces(state.dice);

  return (
    <div className="board-wrap" style={{ 
      display: 'flex', justifyContent: 'center', alignItems: 'center', minHeight: '100vh', 
      background: '#2c3e50', position: 'relative', overflow: 'hidden' 
    }}>
      <svg width="1000" height="760" style={{ background: "transparent", zIndex: 2 }}>
        {state.hexes.map(h => {
          const c = tileCenters.get(h.id);
          return c ? <Tile key={h.id} tile={h} cx={c.x} cy={c.y} size={HEX_SIZE} /> : null;
        })}

        {/* PHYSICAL BANDIT RENDERING - Updated to Bandit Icon, No Circle */}
        {state.robber != null && tileCenters.has(state.robber) && (
          <g transform={`translate(${tileCenters.get(state.robber).x}, ${tileCenters.get(state.robber).y})`}>
            <text 
              textAnchor="middle" 
              dominantBaseline="central" 
              style={{ 
                fontSize: '30px',
                filter: 'drop-shadow(2px 2px 4px rgba(0,0,0,0.5))',
                pointerEvents: 'none' 
              }}
            >
              🥷
            </text>
          </g>
        )}

        {state.edges.map(e => {
          const v1Id = e.vertices ? e.vertices[0] : e.v1;
          const v2Id = e.vertices ? e.vertices[1] : e.v2;
          const a = vertexPos.get(v1Id);
          const b = vertexPos.get(v2Id);
          if (!a || !b) return null;
          const midX = (a.x + b.x) / 2;
          const midY = (a.y + b.y) / 2;
          const port = PORT_DATA[e.port];
          let labelX = midX, labelY = midY, angle = 0;
          if (port) {
            angle = Math.atan2(b.y - a.y, b.x - a.x) * (180 / Math.PI);
            if (angle > 90) angle -= 180;
            if (angle < -90) angle += 180;
            const dx = b.x - a.x, dy = b.y - a.y;
            const len = Math.sqrt(dx * dx + dy * dy);
            let nx = -dy / len, ny = dx / len;
            const toCenterDirX = midX - BOARD_CENTER.x;
            const toCenterDirY = midY - BOARD_CENTER.y;
            if (nx * toCenterDirX + ny * toCenterDirY < 0) { nx = -nx; ny = -ny; }
            labelX = midX + nx * 14;
            labelY = midY + ny * 14;
          }
          return (
            <g key={e.id} onClick={() => onAction({ action: "build_road", edge: e.id })} style={{ cursor: 'pointer' }}>
              <EdgeMarker x1={a.x} y1={a.y} x2={b.x} y2={b.y} edge={e} />
              {port && (
                <text x={labelX} y={labelY} transform={`rotate(${angle}, ${labelX}, ${labelY})`}
                  textAnchor="middle" dominantBaseline="central"
                  style={{ fontSize: '10px', fontWeight: 'bold', fill: '#fff', pointerEvents: 'none', filter: 'drop-shadow(1px 1px 1.5px rgba(0,0,0,0.8))' }}
                >
                  {port.icon} {port.label}
                </text>
              )}
              <line x1={a.x} y1={a.y} x2={b.x} y2={b.y} stroke="black" strokeOpacity={0} strokeWidth={25} />
            </g>
          );
        })}
        {state.vertices.map(v => {
          const p = vertexPos.get(v.id);
          if (!p) return null;
          return (
            <VertexMarker key={v.id} v={{ ...v, x: p.x, y: p.y }} 
              onClick={(e) => { e.stopPropagation(); setSelectedVertex(v.id); setBuildOpen(true); }} 
            />
          );
        })}
      </svg>

      <div style={{ 
        position: 'absolute', right: '50px', top: '50px', zIndex: 100,
        background: isFlashing ? 'rgba(230, 126, 34, 0.6)' : 'rgba(255,255,255,0.15)', 
        padding: '5px 10px', borderRadius: '15px',
        display: 'flex', gap: '15px', fontSize: '2rem', color: isFlashing ? '#f1c40f' : 'white',
        transition: 'all 0.3s ease',
        transform: isFlashing ? 'scale(1.15)' : 'scale(1)',
        boxShadow: isFlashing ? '0 0 30px #e67e22' : 'none',
        pointerEvents: 'none'
      }}>
        <span>{face1}</span>
        <span>{face2}</span>
      </div>

      <BuildDialog
        open={buildOpen} vertexId={selectedVertex} state={state}
        onClose={() => setBuildOpen(false)}
        onBuild={async type => {
          const act = type === "settlement" ? "build_settlement" : "upgrade_city";
          await onAction({ action: act, vertex: selectedVertex });
          setBuildOpen(false);
        }}
      />
      <BanditModal
        open={banditOpen} 
        state={state}
        onPick={async (hexId) => { 
          await onAction({ action: "move_robber", hex: hexId }); 
          setBanditOpen(false); 
        }}
        onClose={() => setBanditOpen(false)}
      />
    </div>
  );
}