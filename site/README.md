# HudBox Website

This is the Docusaurus site that serves as the landing page and documentation for HudBox — a lightweight GTK4 heads‑up display (HUD) window for Linux desktops. The site includes a concise overview, feature highlights, and a single "Getting Started" page mirroring the project README.

Live site (GitHub Pages): https://swstegall.github.io/HudBox/

## Getting Started (Site Development)

Install dependencies:

```bash
yarn
```

Start a local dev server:

```bash
yarn start
```

This starts a local development server and opens a browser. Edits hot‑reload automatically.

## Build the site

```bash
yarn build
```

Outputs static assets to the `build` directory.

## Deploy to GitHub Pages

Using SSH:

```bash
USE_SSH=true yarn deploy
```

Not using SSH:

```bash
GIT_USER=<your GitHub username> yarn deploy
```

This builds the site and pushes the result to the `gh-pages` branch for https://github.com/swstegall/HudBox.
