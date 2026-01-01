// webpack.config.js
const path = require('path');

module.exports = { 
  entry: './src/index.jsx', 
  output: { 
    path: path.resolve(__dirname, 'build'), 
    filename: 'main.js', 
  }, 
  resolve: { 
    extensions: ['.js', '.jsx'], 
  }, 
  module: { 
    rules: [ 
      { 
        test: /\.(js|jsx)$/, 
        exclude: /node_modules/, 
        use: { 
          loader: "babel-loader" 
        } 
      }, 
      { 
        test: /\.css$/, 
        use: ['style-loader', 'css-loader'], 
      }, 
    ] 
  }, 
  devServer: { 
    static: { 
      directory: path.join(__dirname, 'public'), 
    }, 
    compress: true, 
    // CHANGE 1: Move frontend to 3000 so Drogon can have 8080
    port: 3000, 
    open: true,
    // CHANGE 2: Send all API calls to the C++ server
    proxy: [
      {
        context: ['/game', '/api'],
        target: 'http://localhost:8080',
      },
    ],
    historyApiFallback: true 
  }
};
