BEGIN;
CREATE INDEX IF NOT EXISTS idx_users_google ON users(google_id);
CREATE INDEX IF NOT EXISTS idx_matches_start ON matches(start_time);
COMMIT;
