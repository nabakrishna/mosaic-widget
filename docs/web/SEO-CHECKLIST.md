# Mosaic website launch checklist

These files prepare the website for crawling and link previews. They cannot guarantee a search ranking or social reach; useful content, a working public deployment, and genuine recommendations matter too.

## Files in this folder

| File | Purpose |
| --- | --- |
| `head-snippet.html` | Copyable title, description, canonical, social metadata, icons, manifest link, and structured data for the home page. Its metadata is already included in `index.html`. |
| `robots.txt` | Allows crawling and points to the sitemap. |
| `sitemap.xml` | Lists the home, privacy, terms, and selected product images. |
| `site.webmanifest` | Browser and installed-site name, theme, and icon references. |
| `404.html` | Branded not-found page. |
| `favicon.ico` | Browser tab icon, based on the existing Mosaic app icon. |
| `assets/og-image.png` | 1200 × 630 social preview card featuring Mosaic and Labu. |
| `assets/favicon-32.png`, `assets/apple-touch-icon.png`, `assets/icon-192.png`, `assets/icon-512.png` | PNG icons for browser tabs, Apple devices, and Android/installable browsers. |

The absolute URLs currently use `https://nabakrishna.github.io/mosaic-windget/`. This assumes the public site serves the **contents of this `docs/web` folder at that URL**. If the published URL or deployment root changes, update the canonical URL, Open Graph URLs, structured data, `robots.txt`, `sitemap.xml`, and the link in `404.html` together.

## Deploy the website with GitHub Pages

This repository has a GitHub Actions workflow at `.github/workflows/deploy-pages.yml`. It publishes the contents of `docs/web` to `https://nabakrishna.github.io/mosaic-windget/` whenever changes to that folder are pushed to the `main` branch.

### One-time GitHub setup

1. Open the repository on GitHub: [nabakrishna/mosaic-windget](https://github.com/nabakrishna/mosaic-windget).
2. Select **Settings** in the repository navigation.
3. In the left sidebar, open **Pages**.
4. Under **Build and deployment**, set **Source** to **GitHub Actions**. Do not choose “Deploy from a branch”; the workflow deploys the site.
5. In **Settings → Actions → General → Workflow permissions**, leave the general repository permission setting at its default. The Pages workflow declares its own deployment permissions.

### Publish the first version

1. In your local project, save the website changes. The deploy workflow and all website files must be included in the commit.
2. Commit the files and push the commit to the `main` branch. For example, from a terminal in the project folder:

   ```powershell
   git add .github/workflows/deploy-pages.yml docs/web
   git commit -m "Deploy Mosaic website to GitHub Pages"
   git push origin main
   ```

   If Git says there is nothing to commit, check that the website and workflow files have already been committed and pushed.
3. On GitHub, open the repository’s **Actions** tab and select **Deploy website to GitHub Pages**.
4. Open the newest run. Wait for both **build** and **deploy** jobs to complete successfully (green checkmarks). Open a failed job to read its error before retrying.
5. Visit [https://nabakrishna.github.io/mosaic-windget/](https://nabakrishna.github.io/mosaic-windget/). The first publish, or a new DNS/cache update, may take a few minutes.
6. Verify the home page, `privacy.html`, `terms.html`, `404.html`, `robots.txt`, `sitemap.xml`, `assets/og-image.png`, and the icon files load. Confirm the browser is using HTTPS.

For later updates, push changes to `docs/web` on `main`; the workflow runs automatically. To run it manually, select **Actions → Deploy website to GitHub Pages → Run workflow → Run workflow**.

### Before launch

- Update any placeholder contact email in the site with an address you actually monitor.
- Review product claims, download links, privacy/terms information, screenshots, and social captions for accuracy before publishing.
- Keep the canonical URL and social metadata consistent with the final public URL. The `head-snippet.html` block is already present in `index.html`; if you change it, update the copyable snippet too.

## Search engine setup

1. Add the public site to [Google Search Console](https://search.google.com/search-console/) and complete its ownership verification.
2. Submit `https://nabakrishna.github.io/mosaic-windget/sitemap.xml` in Search Console.
3. Use URL Inspection to request indexing of the home page after deployment. Indexing is controlled by Google and is not immediate or guaranteed.
4. Optionally add the site to [Bing Webmaster Tools](https://www.bing.com/webmasters/).
5. Validate structured data with Google's [Rich Results Test](https://search.google.com/test/rich-results) and check page experience with [PageSpeed Insights](https://pagespeed.web.dev/). FAQ markup is included to describe the visible FAQ; Google does not guarantee FAQ rich-result display.

## Useful content and sharing work

- Ensure the main page explains who Mosaic is for, what each widget does, supported Windows versions, and how to download or get help.
- Make screenshot `alt` text specific and useful; use intrinsic `width` and `height` attributes and lazy-load below-the-fold images where appropriate.
- Keep the release link, GitHub repository description, README, and website product information aligned.
- Share the home page with the generated preview image. Check the result with the [Facebook Sharing Debugger](https://developers.facebook.com/tools/debug/) and [LinkedIn Post Inspector](https://www.linkedin.com/post-inspector/); social platforms may cache previews.
- Ask relevant communities for feedback and follow each site's self-promotion rules. Avoid purchased links, keyword stuffing, or fabricated reviews.

## Draft launch captions

**X**

> Meet Mosaic — a customizable desktop widget for Windows with To-Do, Quick Note, Daily Date and Image Box. And a mascot named Labu. Take a look: https://nabakrishna.github.io/mosaic-windget/

**LinkedIn**

> I built Mosaic, a customizable Windows desktop widget for keeping tasks, notes, dates and inspiration close at hand. The site has screenshots and download information: https://nabakrishna.github.io/mosaic-windget/

**Reddit** — Check the community's self-promotion rules first.

> **Title:** I made a customizable Windows desktop widget with tasks, notes and an Image Box — feedback welcome  
> **Body:** Share what motivated you to build Mosaic, what it does, what feedback you want, and links to the website and source repository.

**Hacker News**

> Show HN: Mosaic — customizable desktop widgets for Windows

**Product Hunt**

> **Tagline:** Your desktop, arranged around you.  
> **Description:** Mosaic is a customizable Windows desktop widget for To-Do, Quick Note, Daily Date and Image Box, with Labu as its mascot.
