export async function apiPost(path, body) {
    const res = await fetch(`http://localhost:8080${path}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body)
    });
    return res.json();
}
export async function apiGet(path) {
    const res = await fetch(`http://localhost:8080${path}`);
    return res.json();
}
export async function createMatch({botLevel, playerName}) {
    return apiPost('/game/create', { mode: 'vs_bots', botLevel: botLevel, playerName: playerName});
}
export async function getState(matchId) {
    return apiGet(`/game/${matchId}/state`);
}
export async function doAction(matchId, action) {
    return apiPost(`/game/${matchId}/action`, action);
}
export async function getLeaderboard() {
    return apiGet('/leaderboard');
}
