import React from "react";
export default function Header({ user }) {

    return (
        <div className="header">
            <div className="left">User: {user?.displayName || 'Guest'}</div>
            <div className="center">Catan</div>
            <div className="right"></div>
        </div>
    );
}