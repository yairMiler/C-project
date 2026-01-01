import React, { useEffect, useState } from "react";
import { useNavigate } from "react-router-dom";
import { getLeaderboard } from "../api/api"; // Added this import

export default function LeaderboardPage() {
const [data, setData] = useState([]);
const nav = useNavigate();

useEffect(() => {
    // Defined as an async function to use 'await'
    const fetchData = async () => {
        try {
            const json = await getLeaderboard();
            setData(json);
        } catch (err) {
            console.error(err);
        }
    };
    fetchData();
}, []);

const pageStyle = { 
backgroundColor: "#2c3e50", 
minHeight: "100vh", 
color: "white", 
display: "flex", 
flexDirection: "column", 
alignItems: "center", 
padding: "50px" 
}; 

const tableStyle = { 
width: "80%", 
maxWidth: "800px", 
borderCollapse: "collapse", 
fontSize: "1.2rem", 
marginTop: "30px" 
}; 

return ( 
<div style={pageStyle}> 
<h1 style={{ fontSize: "3rem", color: "#e67e22" }}>🏆 Hall of Fame</h1> 
<table style={tableStyle}> 
<thead> 
<tr style={{ borderBottom: "2px solid #e67e22" }}> 
<th style={{ padding: "15px", textAlign: "left" }}>Player</th> 
<th style={{ padding: "15px" }}>Games</th> 
<th style={{ padding: "15px" }}>Wins</th> 
<th style={{ padding: "15px" }}>Total Points</th> 
</tr> 
</thead> 
<tbody> 
{data.map((user, index) => ( 
<tr key={index} style={{ borderBottom: "1px solid #34495e" }}> 
<td style={{ padding: "15px" }}>{user.name}</td> 
<td style={{ padding: "15px", textAlign: "center" }}>{user.games}</td> 
<td style={{ padding: "15px", textAlign: "center", color: "gold", fontWeight: "bold" }}>{user.wins}</td> 
<td style={{ padding: "15px", textAlign: "center" }}>{user.points}</td> 
</tr> 
))} 
</tbody> 
</table> 
<button 
onClick={() => nav('/')} 
style={{ marginTop: "40px", padding: "10px 30px", cursor: "pointer", borderRadius: "8px" }} 
> 
Back to Home 
</button> 
</div> 
);
}