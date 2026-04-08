# Ghost Session (Beta)

Real-time music collaboration platform. Work on beats together from anywhere — upload stems, arrange tracks, mix, and chat with collaborators in real time.

## Architecture

| App | Stack | Description |
|-----|-------|-------------|
| `apps/desktop` | React, Vite, Tauri, Tailwind | Desktop/web frontend |
| `apps/server` | Hono, Drizzle, Turso, Socket.IO | REST API + WebSocket server |
| `apps/plugin` | JUCE (C++), WebView2 | VST3/AU plugin for DAWs |
| `apps/electron` | Electron | Desktop wrapper |
| `packages/types` | TypeScript | Shared type definitions |
| `packages/tokens` | JSON, Tailwind preset | Design tokens |
| `packages/protocol` | TypeScript | WebSocket protocol types |

## Features

- **Arrangement View** — Timeline-based DAW layout with drag-to-reposition clips
- **Mixer Strip** — Per-track volume faders, mute, and solo
- **Audio Playback** — Synchronized multi-track playback with offset positioning
- **Duplicate & Split** — Copy clips and place them anywhere on the timeline
- **File Upload** — Drag and drop WAV/MP3/FLAC files into projects
- **Real-time Collaboration** — WebSocket-powered live updates, cursors, and chat
- **Video Grid** — See collaborators via WebRTC
- **VST3 Plugin** — Load Ghost Session inside Ableton, FL Studio, Logic, etc.
- **Frequency Visualizer** — Multiple viz modes (bars, wave, radial, ghost particles)
- **Sample Packs** — Organize and share sounds
- **Social Feed** — Share projects with the community

## Setup

```bash
pnpm install
```

### Desktop App (localhost:3000)
```bash
cd apps/desktop
pnpm dev
```

### Server
```bash
cd apps/server
cp .env.example .env  # fill in Turso + R2 credentials
pnpm dev
```

### VST3 Plugin
```bash
# Requires JUCE, CMake, and a C++ compiler
cd apps/plugin
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Environment Variables

### Server (`apps/server/.env`)
- `TURSO_DATABASE_URL` — Turso database URL
- `TURSO_AUTH_TOKEN` — Turso auth token
- `S3_ENDPOINT` — Cloudflare R2 endpoint
- `S3_ACCESS_KEY` / `S3_SECRET_KEY` — R2 credentials
- `S3_BUCKET` — R2 bucket name

### Desktop (`apps/desktop/.env`)
- `VITE_API_URL` — Server API URL
- `VITE_WS_URL` — Server WebSocket URL

## Deployment

Server is deployed on [Railway](https://railway.app). Desktop app connects via the `VITE_API_URL` environment variable.

## License

Proprietary. All rights reserved.
