DO $$ 
BEGIN 
    -- 1. Cleanup old tables (Inside the block so it's one command)
    -- We use 'IF EXISTS' and drop in order of dependencies
    DROP TABLE IF EXISTS tiles;
    DROP TABLE IF EXISTS game_state;
    DROP TABLE IF EXISTS users;
    DROP TABLE IF EXISTS matches;

    -- 2. Create matches table
    CREATE TABLE matches ( 
      id SERIAL PRIMARY KEY, 
      mode VARCHAR(32) NOT NULL, 
      difficulty VARCHAR(32) NOT NULL, 
      start_time TIMESTAMP DEFAULT now(), 
      end_time TIMESTAMP, 
      winner_name VARCHAR(128) 
    ); 

    -- 3. Create users table (UNIQUE is now applied here)
    CREATE TABLE users ( 
      display_name VARCHAR(128) UNIQUE NOT NULL, 
      games_played INT DEFAULT 0, 
      wins INT DEFAULT 0, 
      total_points INT DEFAULT 0, 
      created_at TIMESTAMP DEFAULT now() 
    ); 

    -- 4. Create game_state table
    CREATE TABLE game_state ( 
      match_id INT PRIMARY KEY REFERENCES matches(id), 
      json_state JSONB NOT NULL 
    ); 

    -- 5. Create tiles table
    CREATE TABLE tiles ( 
      id SERIAL PRIMARY KEY, 
      match_id INT REFERENCES matches(id), 
      type VARCHAR(16), 
      number INT, 
      x INT, 
      y INT 
    ); 
END $$;
