# QZN Protocol — Quorum Incubation Proposal
### March 2026

**Hunter Duttenhefer · Sole Founder · Qubic Ambassador**

> *"If I can bring Solana devs or Ethereum devs to the network with QZN, I'll do it in a heartbeat. This chain has the potential of a lot of things."*
> — Hunter Duttenhefer, Founder, QZN Protocol

---

## The Ask

**30B QU** — routed in full to **mundus_tj85 (Mundus)**, an established Qubic ecosystem auditor who has already reviewed the QZN codebase and issued a formal quote.

Six interdependent contracts, 10,172 lines of C++, and 400+ passing tests represent a deeply interconnected system that requires a complete audit, not a partial one. The 30B QU ask covers the full audit plus a re-audit reserve — ensuring completion without requiring a second Quorum proposal.

This is not an operational ask. This is ecosystem insurance. If QZN ships buggy contracts, it does not just damage QZN — it damages Qubic's entire smart contract credibility. The audit protects every computor on this network, not just this project.

---

## What Has Been Built

**6 production-grade C++ smart contracts. 10,172 lines. 400+ passing tests. Running in Docker CI today.**

This is not a whitepaper. This is working code.

| Contract | Index | Role |
|---|---|---|
| QZN_Token_v2 | 26 | Native currency — required by all other contracts |
| QZN_GameCabinet_PAO | 27 | Modular game registry — plugs any PAO game in |
| QZN_RewardRouter_PAO | 28 | Atomic fee distribution across all 7 parties |
| QZN_TreasuryVault_PAO | 29 | Capital management, prize backstop, development fund |
| QZN_Portal_PAO | 30 | 16-node liquidity and infrastructure layer |
| QZN_TournamentEngine_PAO | 31 | Competitive brackets, prize structures, leaderboards |

The three launch games — **snaQe**, **paQman**, and **TANQ-BATTLE** — are registered through QZN_GameCabinet_PAO. PAO stands for Programmable Arcade Organization.

All QZN contract logic, tests, and build scripts are original work. The repository builds on Qubic's open-source qubic-core-lite base layer — standard practice for all Qubic smart contract development.

**Repository:** [github.com/dutt9145/qzn-core-lite](https://github.com/dutt9145/qzn-core-lite)  
**Frontend:** [qzn.app](https://qzn.app)

---

## Deterministic Onchain Economics

Every major gaming platform runs on discretionary economics. Axie manually adjusted SLP emission rates multiple times and collapsed the token from $0.35 to $0.004. No player, staker, or liquidity provider can know what they will receive — until now.

Every economic outcome in QZN is encoded immutably and executes identically on every transaction. No human discretion at the transaction level. No vote, no manual trigger, no override. PAO governance exists for higher-order decisions like game additions and treasury proposals, but it cannot touch the core economic routing. The distribution is law. The governance is a feature built on top of it.

**This is the routing on every single game entry fee:**

| Destination | BPS | % |
|---|---|---|
| Prize Pool | 4,500 | 45% |
| Stakers | 2,000 | 20% |
| Liquidity Providers | 2,000 | 20% |
| Burn | 1,000 | 10% |
| Nodes | 300 | 3% |
| Treasury | 100 | 1% |
| QSWAP Protocol | 100 | 1% |

**The burn line:** Every game entry permanently removes QU from supply. At scale, QZN becomes a deflationary pressure engine for the entire network.

QZN's RewardRouter_PAO is fully open-source, auditable on-chain, and distributes to seven parties atomically on every transaction — by design, not by discretion.

---

## QZN as a Reusable Economic Layer

These 6 contracts are not 6 products. They are a composable economic stack.

snaQe, paQman, and TANQ-BATTLE plug into a shared PAO backbone — they do not each carry their own token or treasury. Every game built after launch inherits the same economic rails automatically. The token appreciates. Stakers earn more. Liquidity deepens. Burn accelerates. All from one layer.

Most crypto game tokens are single-game instruments. When the game dies, the token dies. QZN survives any individual game because **the layer is the product**.

QSWAP has a dedicated 100 BPS line baked into every transaction — meaning QZN and QSWAP grow together by design. Standard constant-product AMMs spread liquidity uniformly across all price ranges, leaving the majority of capital idle at any given price point. This forces LPs to charge elevated fees to compensate for impermanent loss — punishing both traders and liquidity providers. DLMM concentrates liquidity into active price bins, cutting slippage and fees dramatically. For a Qubic-native DEX in early-stage TVL conditions, capital efficiency is not a nice-to-have — it is survival.

After Mainnet, DLMM performance data will be validated through a proprietary prediction market flywheel — ensuring liquidity parameters are optimized for real market conditions from day one.

---

## Why 6 Contracts Deploy Together

This is a technical constraint confirmed by **JoeTom (Qubic core developer)**, not a preference.

- **Token_v2 must sit at index 26.** Every other contract makes cross-contract calls to it. This is a Qubic protocol requirement.
- **Hard dependency chain.** GameCabinet calls Token. RewardRouter calls both. TournamentEngine calls RewardRouter and TreasuryVault. Stubs cannot hold indices.
- **The system is the MVP.** A GameCabinet without RewardRouter means players win but no one gets paid. Partial deployment is not a product — it is a broken system.

Staggered deployment across multiple epochs has been proposed to mundus_tj85 to address network-load concerns. The contracts still deploy as a single Quorum proposal per JoeTom's guidance.

---

## Existing Demand

The mechanics have been proven by billions of players over 40 years — without any economic incentive.

| Game | Mechanic | Historical Reach |
|---|---|---|
| snaQe | Snake | 400M+ Nokia players alone |
| paQman | Pac-Man | $14B+ lifetime franchise revenue; 30M+ monthly mobile players in 2022 |
| TANQ-BATTLE | Tank Battle | Top-10 NES game by playtime; top-charting remakes for 3 decades |

Nokia Snake had 400 million players with no prize pools, no staking, and no tournament brackets. Pac-Man generated $14 billion in franchise revenue before the internet existed. Tank Battle has been remade dozens of times across 40 years because people never stopped wanting to play it. None of those players were earning anything. They played because the mechanics are addictive by design.

QZN adds something none of them ever had — a reason beyond fun. Every game entry feeds a prize pool. Every token held earns staking returns. Every tournament bracket puts real QU on the line. The mechanics are already proven. The economic layer on top of them is entirely new.

This is not the crypto gaming thesis of "we built a game, now find players." The players already exist. They have existed for four decades. QZN is built to meet them with infrastructure worthy of what they already love.

And this is only the beginning. snaQe, paQman, and TANQ-BATTLE are the launch slate — not the ceiling. Every game built on QZN after launch inherits the same economic rails. The player base compounds. The burn accelerates. The token appreciates. The reusable layer pays dividends on every title that comes after it.

The demand is not hypothetical. It is a 40-year empirical record — QZN is built to capture it on-chain.

---

## Protocol Economics

> All figures use a 500K QU standard entry fee and 30 game sessions per active player per month. Volume is denominated in QU.

### ICO Structure — Total Fixed Supply: 250M QZN

| Phase | Supply | Price | Gross Raise |
|---|---|---|---|
| Phase 1 | 12.5M QZN | 150 QU | 1.875B QU |
| Phase 2 | 50M QZN | 337 QU | 16.85B QU |

ICO proceeds cover the audit, infrastructure, and marketing runway before a single game entry is processed.

### Post-Launch QU Velocity Forecast

| Milestone | Players | Monthly Volume | Protocol Revenue (55%) | QU Burned (10%) |
|---|---|---|---|---|
| Month 1 | 200 | 3B QU | 1.65B QU | 300M QU |
| Month 3 | 500 | 7.5B QU | 4.125B QU | 750M QU |
| Month 6 | 1,500 | 22.5B QU | 12.375B QU | 2.25B QU |
| Month 12 | 5,000 | 75B QU | 41.25B QU | 7.5B QU |
| Year 2 | 15,000 | 225B QU | 123.75B QU | 22.5B QU |

At Qubic's prior all-time high of $0.0000124 per QU — approximately 12x current price — every QU figure above multiplies accordingly. Zero protocol changes, zero new games, zero new players required.

### Node Revenue — Month 12 Base Case

| Node Type | Count | Pool Share | Per Node / Month | Per Node / Year |
|---|---|---|---|---|
| Genesis Nodes | 10 | 65% | 146.25M QU | 1.755B QU |
| Core Nodes | 6 | 35% | 131.25M QU | 1.575B QU |

Node operators earn passive QU income from every game played — regardless of outcome, regardless of which game, and regardless of token price. The 16 Portal nodes will be offered via OTC following Quorum approval. Once sold, node operators will have direct financial incentive to see QZN succeed and to advocate for this proposal.

---

## Who Is Building This

Hunter Duttenhefer is the sole founder of QZN Protocol. He is a credentialed Medical Laboratory Scientist with six years of clinical experience, an MBA in Finance, a self-taught Qubic QPI/C++ smart contract developer, and an official Qubic Ambassador — a participant in Qubic's official ambassador program.

All 6 QZN smart contracts, the test suite, Docker CI environment, and deployment infrastructure were written solely by Hunter Duttenhefer. The repository builds on Qubic's open-source qubic-core-lite base layer — the standard foundation for all Qubic smart contract development. The secondary contributor shown on GitHub reflects inherited base layer credentials from the original server configuration, not updated to the founder's credentials until final deployment. Every line of QZN contract logic, every test, and every build script is original work.

**Ecosystem relationships in place:**

- ✅ **Qubic Ambassador** — Qubic's Official Ambassador Program
- ✅ **DeFiMomma** — Qubic Marketing Lead, connected QZN directly to the Incubation Program
- ✅ **JoeTom** — Core dev, confirmed 6-contract single-proposal approach and ascending index requirement
- ✅ **mundus_tj85** — Formal 25B QU audit quote in hand, actively engaged
- ✅ **Spike / SpikeinJapan** — PORTAL/QSWAP lead, DLMM collaboration pathway open
- ✅ **Mr. Rose** — Incubation Reviewer, actively engaged in QZN proposal review

---

*We build the platform and let the people come.*

**Contact:** hello@qzn.app · [qzn.app](https://qzn.app) · [github.com/dutt9145/qzn-core-lite](https://github.com/dutt9145/qzn-core-lite)
