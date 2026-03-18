# QZN Core-Lite Node

Deploys qubic/core-lite to Railway as a local testnet single node for testing QZN smart contracts.

## Setup

### 1. Add your QZN contracts
Drop all 6 contract `.h` files into the `contracts/` folder.

### 2. Push to GitHub
git init
git add .
git commit -m "init qzn core-lite"
git remote add origin https://github.com/YOUR_USERNAME/qzn-core-lite.git
git push -u origin main

### 3. Deploy to Railway
1. New Project → Deploy from GitHub repo → select qzn-core-lite
2. Railway auto-detects the Dockerfile and builds on Linux x86

### 4. Access the RPC
https://your-service.up.railway.app/live/v1   → node status
https://your-service.up.railway.app/          → stats
https://your-service.up.railway.app/query/v1  → query contracts

## Notes
- Runs in local testnet mode (single node devnet, no mainnet connection)
- 676 test seeds each pre-funded with 10B QU for testing
- Default epoch duration is 3000 ticks# qzn-core-lite
