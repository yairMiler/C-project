import React from "react";

export function hexPoints(cx, cy, size) {
  const pts = [];
  for (let i = 0; i < 6; i++) {
    const a = Math.PI / 180 * (60 * i - 30);
    pts.push({
      x: cx + size * Math.cos(a),
      y: cy + size * Math.sin(a),
    });
  }
  return pts;
}

function terrainColor(t) {
  return [
    "#c27f3b", "#2e8b57", "#9b8f8f",
    "#f5e79e", "#b2d08e", "#e8d7b5"
  ][t] ?? "#ddd";
}

export default function Tile({ tile, cx, cy, size }) {
  const pts = hexPoints(cx, cy, size);

  return (
    <g>
      <polygon
        points={pts.map(p => `${p.x},${p.y}`).join(" ")}
        fill={terrainColor(tile.terrain)}
        stroke="#5a4"
        strokeWidth={2}
      />

      {tile.token !== 0 && (
        <>
          <circle cx={cx} cy={cy} r={18} fill="#fff" stroke="#444" />
          <text x={cx} y={cy + 5} textAnchor="middle">{tile.token}</text>
        </>
      )}
    </g>
  );
}





