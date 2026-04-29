# QZN Protocol — Incubation Program Application v2

*Hunter Duttenhefer · Qubzylthar Nexus LLC · April 2026*

---

## 1. Project Description

QZN Protocol is an on-chain arcade gaming layer on Qubic. Players stake QU to play games, and every match routes value through a single token contract that distributes prizes to winners, revenue to QZN node owners, dividends to stakers, and fees to the treasury.

**What's already built:**

- **Six Smart Contracts** — 10,172 lines of C++, 436 of 436 tests passing
- **Three Live Games** — snaQe, paQman, TANQ-BATTLE — with dual-signature settlement (server signs, then player countersigns) to prevent both server manipulation and player cheating
- **Frontend** at qzn.app, **Backend** at api.qzn.app
- **QZNNODES Integration** (100-node supply, 3-tier revenue model)
- **Multi-Sig Treasury** via the TreasuryVault contract

**This isn't a project asking for capital to start. It's a project asking for capital to finish.**

### What changed from v1 → v2

The board flagged three things in v1. v2 fixes all three.

| v1 issue | v2 fix |
| --- | --- |
| Genesis burn was confusing | No burn. The 12.5M QZN that would have burned is now in Treasury under multi-sig governance. |
| Unit economics weren't defensible | Full 7-bucket per-match routing now published in Section 3, matching live contract: 45/20/20/10/3/1/1. Hardcoded as constexpr in Token_v2. |
| Funding picture was vague | Funding is fully stacked. 10B board + 10B founder/ICO. |

**Ask shrunk 67% (30B → 10B). Listening to the board was the whole point.**

---

## 2. Technology Stack

| Contract | What it does | LOC |
| --- | --- | --- |
| Token_v2 | Routing hub — every QZN transfer flows through it | 541 |
| GameCabinet | Player wagers, dual-signature settlement | 3,386 |
| RewardRouter | Prize distribution, staking rewards, leaderboard | 2,546 |
| TreasuryVault | 3-signer multi-sig, governance proposals | 1,453 |
| TournamentEngine | Bracket play, prize pools | 1,178 |
| QZNNODES | QZN node ownership, access tiers | 1,068 |

- **Frontend:** Next.js / TypeScript / Tailwind, deployed on Vercel
- **Backend:** Node.js on Railway, Supabase for data
- **Asset issuance:** QubicTrade (creates the 250M QZN as a tradeable on-chain asset)
- **ICO launch:** QIP — Qubic ICO Portal
- **Repository:** github.com/dutt9145/qzn-core-lite (open source)

---

## 3. Business Plan

### Token allocation

| Bucket | Tokens | Percentage | Notes |
| --- | --- | --- | --- |
| Treasury | 87.5M | 35% | Multi-sig governed (2-of-3 post-upgrade) |
| ICO | 50M | 20% | 3 phases — see ICO plan below |
| Team | 50M | 20% | 1-yr cliff, 5-yr linear vest, hardcoded |
| Liquidity | 37.5M | 15% | QSWAP pool seed |
| Ecosystem | 25M | 10% | See Ecosystem Allocation table below |
| **Total** | **250M** | **100%** | |

**Ecosystem Allocation (25M QZN)**

| Sub-Bucket | Tokens | Purpose |
| --- | --- | --- |
| Third-Party Gaming Developer Grants | 10M | PAO deployment incentives, milestone-based |
| Tournament Seed Pools | 5M | TournamentEngine prize seeding for first 12 months |
| Bug Bounty Program | 3M | Ongoing post-audit security coverage |
| Strategic Partnerships / Listings | 5M | QSWAP integration, exchanges |
| Marketing / Community / Ambassadors | 2M | Player acquisition, content, programs |
| **Total** | **25M** | |

**250M QZN total.** Team allocation locked behind a 1-year cliff and 5-year linear vest, coded into Token_v2 and not changeable post-deployment.

*Note: protocol session burns are coded into contracts as a separate supply-reduction mechanism. They benefit holders ecosystem-wide but are not part of the board repayment.*

### How the protocol generates value

Every game session routes QZN through the token contract using fixed percentages set in code. A typical match routes:

**Per-Match Capital Distribution**

| Bucket | Percentage |
| --- | --- |
| Prize Pool | 45% |
| Staker Yield | 20% |
| QSWAP Liquidity | 20% |
| Burn | 10% |
| QZNNODES | 3% |
| QSWAP Protocol | 1% |
| QZN Protocol Treasury | 1% |
| **TOTAL** | **100%** |

*These percentages are hardcoded as constexpr values in Token_v2 and are not changeable post-deployment.*

### ICO plan

| Phase | Tokens | Price | Raise |
| --- | --- | --- | --- |
| Phase 1 | 12.5M QZN | 250 QU | 3.125B QU |
| Phase 2 | 25.0M QZN | 400 QU | 10.0B QU |
| Phase 3 | 12.5M QZN | 550 QU | 6.875B QU |
| **Total** | **50M QZN** | — | **20B QU** |

*3-phase structure aligns with QIP's standard ICO format. Stepped pricing reflects risk retirement between phases: Phase 1 buyers commit pre-launch with founder backstop and accept the highest uncertainty in exchange for the lowest entry price. Each tier is priced to reflect the de-risked state of the protocol at that phase, not arbitrary appreciation.*

3.5B QU is already reserved for QubicTrade issuance fees and QIP listing costs. That's earmarked separately and not part of this ask.

---

## 4. Team Overview

### Hunter Duttenhefer — Founder, Lead Smart Contract Developer

Credentialed Qubic Ambassador. ASCP-certified Medical Laboratory Scientist. MBA in Finance (May 2025).

Built the entire QZN smart contract suite — 10,172 lines of production C++ across six contracts — solo, while maintaining an active clinical career. Stack covers Qubic QPI/C++, Next.js/TypeScript, Railway/Supabase, SQL/Power BI, and algorithmic trading systems. The MBA underpins the economic architecture: BPS routing, ICO pricing, tokenomics, treasury governance — all designed in-house, not outsourced.

### Pham (pctsvn) — Core Frontend Developer

Owns the frontend architecture and mobile responsiveness of qzn.app. Holds Developer access to the QZN Vercel deployment and manages PR approvals on the primary frontend repo under structured branch protection (feature/* → dev → main). Frontend coverage includes game pages, staking dashboard, ICO (Ticker-$QZN), leaderboard, rewards, epochs, whitepaper, QZN node registration, and QSWAP integration. Pham is the project's second independent technical contributor — frontend deployment isn't a single point of failure.

### Ecosystem Advisors

| Name | Role | Contribution |
| --- | --- | --- |
| JoeTom | Qubic core developer | Confirmed the six-contract PR approach; coordinating mainnet deployment at indices 26–31 |
| E | Founder, QIP & Qusino | ICO structure and pricing strategy; QIP launch coordination |
| DeFiMomma | Ecosystem connector | Introduced QZN to incubation; community credibility anchor |
| Spike (SpikeinJapan) | PORTAL/QSWAP lead | Integration partner; multi-sig Key 2 upon governance upgrade |

---

## 5. Milestone-based Disbursement

### Audit Partner: Mundus (mundus_tj85)

Mundus is the appointed auditor for the QZN smart contract suite. Formal quote of 20B QU for full six-contract scope, negotiated from an initial 25B QU. Engagement begins on M1 milestone disbursement (signed Statement of Work, or SOW). Mundus is independent of the development team and operates under a fixed-scope, fixed-fee contract — no equity, no token allocation, no ongoing relationship beyond audit deliverables.

### Disbursement schedule

**10B QU disbursed in 4 equal milestone tranches**

| Tranche | Amount | Testable deliverable | Week |
| --- | --- | --- | --- |
| M1 — Audit engaged | 2.5B | Signed audit SOW with Mundus, kickoff document published publicly | Week 1 |
| M2 — Preliminary findings | 2.5B | Mundus publishes interim report covering GameCabinet | Week 2–3 |
| M3 — Full draft report | 2.5B | Mundus publishes full draft covering GameCabinet + RewardRouter | Week 4–6 |
| M4 — Final report public | 2.5B | Final audit report public, remediation verified, ICO asset ready for QubicTrade issuance | Pre-ICO (Early June) |

The 10B disburses in four tranches, each tied to a testable deliverable that Qubic QA can verify. Payment on delivery — no upfront.

**Token_v2** is nominated for the Qubic Incubation Program's covered audit slot (1 contract per project). It's the routing hub — every transfer in the system flows through it — so it's the highest-leverage audit target and the strongest possible use of the free coverage.

**GameCabinet** and **RewardRouter** (the two largest user-facing contracts) get private Mundus audit, funded by this proposal.

---

## 6. Budget Breakdown

Total audit cost is **20B QU** (Mundus, full scope). Asking the board for **10B**. Founder + ICO covers the other 10B. **50/50 split — the board never carries it alone.**

### Source of funds (audit)

| Source | Amount | Notes |
| --- | --- | --- |
| Incubation Board (this ask) | 10B QU | Disbursed across M1–M4 milestones |
| Qubic free audit slot | 1 contract | Token_v2 covered under Incubation Program |
| ICO proceeds + founder personal | 10B QU | Deployed as needed; founder personally backstops if ICO undersubscribes |
| **Total funded** | **20B QU** | Fully covered, no gaps |

### Repayment

**100% repayment in QU within 24 months of ICO close. No interest.**

| Mechanism | Amount | Timing |
| --- | --- | --- |
| Initial repayment installment | 1B QU (5% of gross ICO proceeds) | Within 60 days of ICO Phase 3 close. Off the top. Non-discretionary. |
| Monthly repayment | 375M / month | Begins 60 days after ICO end (24 months) |

If the project succeeds faster, the board gets paid back faster. If it doesn't, the founder absorbs the timeline. Never a default.

### Treasury governance

Treasury launches under founder custody with publicly defined hard caps and full on-chain transparency. Every transaction over 100M QU gets announced in QZN Discord and logged in a pinned public ledger.

**Multi-sig upgrade is committed before ICO Phase 3 opens** — 2-of-3 structure, with the Community Elect keyholder filled by public governance vote. This isn't deferred accountability; it's honest sequencing of trust. We won't appoint keyholders we can't vouch for just to check a box.

---

## 7. Commitment

The founder personally commits to QZN as a primary professional commitment for a minimum of one year post-ICO, dedicating active development and maintenance attention during that period. This is a personal time-allocation commitment from the founder, distinct from the LLC's financial obligations to the board described below, which run on a 24-month repayment schedule.

**Specific obligations:**

- **Audit completion** in time for June 2026 ICO
- **Multi-sig treasury upgrade** before ICO Phase 3 opens
- **Quarterly board reports** on repayment, revenue, and project health
- **Open-source codebase** maintained at github.com/dutt9145/qzn-core-lite
- **Public on-chain transparency** — every material treasury action logged before execution
- **Repayment commitment** — 10B QU repaid in full within 24 months of ICO close
- **No founder withdrawals from treasury** until ICO Phase 3 closes
- **Founder personal portfolio remains in the Qubic ecosystem** — meaning founder financial outcomes are tied to ecosystem health, not extracted from it

The founder isn't building QZN to launch and exit. The plan is to operate, maintain, and grow QZN as Qubic's flagship gaming vertical for years. Repaying the board on time, returning value to the ecosystem, and continuing to ship are how we earn the next round of trust — and the next ask, when we earn one.

### Continuity and operational resilience

QZN is currently founder-led on the smart contract layer, and the proposal reflects that honestly. Continuity is structured at the system level rather than depending on any single individual:

- **Treasury survives founder unavailability.** Post-Phase 2 multi-sig (2-of-3, with Spike as Key 2 and Community Elect as Key 3) means treasury operations continue without the founder key. No single keyholder can move funds; no single keyholder being unavailable freezes the protocol.
- **Codebase is operable by any qualified developer.** All six contracts are open-source at github.com/dutt9145/qzn-core-lite, audited by Mundus, and documented at the test level (436 of 436 GTests passing serve as executable specifications). The Mundus audit deliverables become permanent technical reference material.
- **Legal entity is durable.** Qubzylthar Nexus LLC (Wyoming) holds all protocol rights, contracts, and treasury accountability independent of any individual. Repayment obligations to the board attach to the LLC, not the founder personally — meaning the obligation, and the entity's accountability for it, survives founder availability.
- **Frontend has independent maintainership.** Pham (pctsvn) holds Developer access on Vercel and PR approval rights on the primary repo, ensuring the public-facing protocol surface remains operational without founder involvement.
- **In the event of extended founder unavailability**, the LLC's operating agreement designates successor authority. The board would receive notice within 30 days and a continuity plan — including any required adjustments to the repayment schedule — within 60 days. Repayment is not waived; it is restructured if necessary, with the same total principal owed.

---

## What happens if approved

| Day | Action |
| --- | --- |
| 0 | Board approval received |
| 7 | Mundus SOW signed |
| 14 | Audit Phase 1 kicks off |
| 30–60 | Audit deliverables published |
| Pre-ICO | Final report public, ICO ready |
| 90 | First quarterly report to board |

---

## Legal posture

Qubzylthar Nexus LLC (Wyoming) · EIN on file · NAICS 511210

All flows route through the LLC. Repayment auditable on both sides.

---

## Contact

**Hunter Duttenhefer** · @Dutte on Discord · hello@qzn.app

Thank you to the board, and to Mr. Rose specifically, for the v1 feedback. This proposal exists because that feedback was clear and actionable.
