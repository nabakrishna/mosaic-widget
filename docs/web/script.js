/* =========================================================
   MOSAIC — INTERACTION & MOTION ENGINE
   ========================================================= */

document.addEventListener("DOMContentLoaded", () => {
  "use strict";

  /* ---------------------------------------------------------
     GLOBALS
  --------------------------------------------------------- */

  const reduceMotion = window.matchMedia(
    "(prefers-reduced-motion: reduce)"
  ).matches;

  const cursorGlow = document.querySelector(".cursor-glow");
  const progress = document.querySelector(".scroll-progress");
  const heroStage = document.querySelector(".hero-stage");

  /* ---------------------------------------------------------
     MOBILE NAV TOGGLE
  --------------------------------------------------------- */

  const navToggle = document.getElementById("navToggle");
  const navMenu = document.getElementById("navLinks");

  if (navToggle && navMenu) {
    function closeNav() {
      navMenu.classList.remove("open");
      navToggle.setAttribute("aria-expanded", "false");
    }

    navToggle.addEventListener("click", () => {
      const isOpen = navMenu.classList.toggle("open");
      navToggle.setAttribute("aria-expanded", String(isOpen));
    });

    navMenu.addEventListener("click", (event) => {
      if (event.target.closest("a")) closeNav();
    });

    document.addEventListener("click", (event) => {
      if (
        navMenu.classList.contains("open") &&
        !navMenu.contains(event.target) &&
        !navToggle.contains(event.target)
      ) {
        closeNav();
      }
    });

    document.addEventListener("keydown", (event) => {
      if (event.key === "Escape") closeNav();
    });
  }

  /* ---------------------------------------------------------
     SCREENSHOT TRACK
  --------------------------------------------------------- */

  const screenshotsMarquee =
    document.querySelector(".screenshots-marquee");
  const screenshotsTrack =
    screenshotsMarquee?.querySelector(".screenshots-track");

  if (screenshotsMarquee && screenshotsTrack) {
    let offset = 0;
    let loopWidth = 0;
    let pointerId = null;
    let previousPointerX = 0;
    let isHovered = false;
    let isDragging = false;
    let interactionTimer = null;
    let previousFrameTime = 0;

    function measureScreenshotTrack() {
      const firstItem = screenshotsTrack.firstElementChild;
      const firstRepeatedItem =
        screenshotsTrack.querySelector('[aria-hidden="true"]');

      if (firstItem && firstRepeatedItem) {
        loopWidth =
          firstRepeatedItem.getBoundingClientRect().left -
          firstItem.getBoundingClientRect().left;
      } else {
        loopWidth = Math.max(
          0,
          screenshotsTrack.scrollWidth -
            screenshotsMarquee.clientWidth
        );
      }
    }

    function updateScreenshotTrack() {
      if (loopWidth > 0) {
        while (offset <= -loopWidth) offset += loopWidth;
        while (offset > 0) offset -= loopWidth;
      } else {
        offset = 0;
      }
      screenshotsTrack.style.transform =
        `translate3d(${offset}px, 0, 0)`;
    }

    measureScreenshotTrack();
    updateScreenshotTrack();

    function pauseAfterInteraction() {
      window.clearTimeout(interactionTimer);
      interactionTimer = window.setTimeout(() => {
        interactionTimer = null;
      }, 900);
    }

    function animateScreenshotTrack(timestamp) {
      if (!previousFrameTime) previousFrameTime = timestamp;
      const elapsed = timestamp - previousFrameTime;
      previousFrameTime = timestamp;

      if (!isHovered && !isDragging && !interactionTimer && loopWidth > 0) {
        offset -= elapsed * 0.042;
        updateScreenshotTrack();
      }

      if (!reduceMotion) {
        window.requestAnimationFrame(animateScreenshotTrack);
      }
    }

    if (!reduceMotion) {
      window.requestAnimationFrame(animateScreenshotTrack);
    }

    screenshotsMarquee.addEventListener("pointerenter", (event) => {
      if (event.pointerType === "mouse") isHovered = true;
    });
    screenshotsMarquee.addEventListener("pointerleave", (event) => {
      if (event.pointerType === "mouse") isHovered = false;
    });

    screenshotsMarquee.addEventListener(
      "wheel",
      (event) => {
        if (loopWidth <= 0 || event.deltaY === 0) return;

        event.preventDefault();
        offset -= event.deltaY;
        updateScreenshotTrack();
        pauseAfterInteraction();
      },
      { passive: false }
    );

    screenshotsMarquee.addEventListener(
      "pointerdown",
      (event) => {
        if (event.pointerType === "mouse" && event.button !== 0) return;

        pointerId = event.pointerId;
        isDragging = true;
        window.clearTimeout(interactionTimer);
        interactionTimer = null;
        previousPointerX = event.clientX;
        screenshotsMarquee.setPointerCapture(pointerId);
      }
    );

    screenshotsMarquee.addEventListener(
      "pointermove",
      (event) => {
        if (event.pointerId !== pointerId) return;

        offset += event.clientX - previousPointerX;
        previousPointerX = event.clientX;
        updateScreenshotTrack();
      }
    );

    function stopScreenshotDrag(event) {
      if (event.pointerId !== pointerId) return;
      pointerId = null;
      isDragging = false;
    }

    screenshotsMarquee.addEventListener(
      "pointerup",
      stopScreenshotDrag
    );
    screenshotsMarquee.addEventListener(
      "pointercancel",
      stopScreenshotDrag
    );
    window.addEventListener(
      "resize",
      () => {
        measureScreenshotTrack();
        updateScreenshotTrack();
      },
      { passive: true }
    );
  }

  /* ---------------------------------------------------------
     CURSOR GLOW
  --------------------------------------------------------- */

  if (cursorGlow && !reduceMotion) {
    let mouseX = window.innerWidth / 2;
    let mouseY = window.innerHeight / 2;

    let glowX = mouseX;
    let glowY = mouseY;

    window.addEventListener(
      "pointermove",
      (event) => {
        mouseX = event.clientX;
        mouseY = event.clientY;
      },
      { passive: true }
    );

    const animateGlow = () => {
      glowX += (mouseX - glowX) * 0.12;
      glowY += (mouseY - glowY) * 0.12;

      cursorGlow.style.left = `${glowX}px`;
      cursorGlow.style.top = `${glowY}px`;

      requestAnimationFrame(animateGlow);
    };

    animateGlow();
  }

  /* ---------------------------------------------------------
     SCROLL PROGRESS
  --------------------------------------------------------- */

  function updateScrollProgress() {
    if (!progress) return;

    const documentHeight =
      document.documentElement.scrollHeight - window.innerHeight;

    if (documentHeight <= 0) {
      progress.style.width = "0%";
      return;
    }

    const percentage =
      (window.scrollY / documentHeight) * 100;

    progress.style.width = `${Math.min(100, percentage)}%`;
  }

  window.addEventListener(
    "scroll",
    updateScrollProgress,
    { passive: true }
  );

  updateScrollProgress();

  /* ---------------------------------------------------------
     SCROLL REVEAL
  --------------------------------------------------------- */

  const revealElements = document.querySelectorAll(
    "[data-reveal], .reveal-card"
  );

  if ("IntersectionObserver" in window) {
    const revealObserver = new IntersectionObserver(
      (entries, observer) => {
        entries.forEach((entry) => {
          if (!entry.isIntersecting) return;

          entry.target.classList.add("visible");

          observer.unobserve(entry.target);
        });
      },
      {
        threshold: 0.12,
        rootMargin: "0px 0px -8% 0px",
      }
    );

    revealElements.forEach((element, index) => {
      element.style.transitionDelay =
        `${Math.min(index * 55, 300)}ms`;

      revealObserver.observe(element);
    });
  } else {
    revealElements.forEach((element) => {
      element.classList.add("visible");
    });
  }

  /* ---------------------------------------------------------
     3D TILT
  --------------------------------------------------------- */

  function createTilt(element, strength = 8) {
    if (reduceMotion) return;

    let targetX = 0;
    let targetY = 0;

    let currentX = 0;
    let currentY = 0;

    let animationFrame = null;

    function animateTilt() {
      currentX += (targetX - currentX) * 0.10;
      currentY += (targetY - currentY) * 0.10;

      element.style.setProperty(
        "--tilt-x",
        `${currentY}deg`
      );

      element.style.setProperty(
        "--tilt-y",
        `${currentX}deg`
      );

      element.style.transform =
        `perspective(1000px)
         rotateX(${currentY}deg)
         rotateY(${currentX}deg)`;

      animationFrame = requestAnimationFrame(
        animateTilt
      );
    }

    element.addEventListener(
      "pointermove",
      (event) => {
        const rect =
          element.getBoundingClientRect();

        const x =
          (event.clientX - rect.left) /
          rect.width;

        const y =
          (event.clientY - rect.top) /
          rect.height;

        targetX =
          (x - 0.5) * strength;

        targetY =
          (0.5 - y) * strength;

        if (!animationFrame) {
          animateTilt();
        }
      }
    );

    element.addEventListener(
      "pointerleave",
      () => {
        targetX = 0;
        targetY = 0;
      }
    );
  }

  document
    .querySelectorAll("[data-tilt]")
    .forEach((element) => {
      createTilt(element, 7);
    });

  /* ---------------------------------------------------------
     MAGNETIC BUTTONS
  --------------------------------------------------------- */

  document
    .querySelectorAll("[data-magnetic]")
    .forEach((button) => {

      if (reduceMotion) return;

      let currentX = 0;
      let currentY = 0;

      let targetX = 0;
      let targetY = 0;

      let frame = null;

      function animateMagnetic() {
        currentX +=
          (targetX - currentX) * 0.15;

        currentY +=
          (targetY - currentY) * 0.15;

        button.style.transform =
          `translate3d(
            ${currentX}px,
            ${currentY}px,
            0
          )`;

        frame = requestAnimationFrame(
          animateMagnetic
        );
      }

      button.addEventListener(
        "pointermove",
        (event) => {

          const rect =
            button.getBoundingClientRect();

          targetX =
            (event.clientX -
              (rect.left + rect.width / 2)) *
            0.12;

          targetY =
            (event.clientY -
              (rect.top + rect.height / 2)) *
            0.12;

          if (!frame) {
            animateMagnetic();
          }
        }
      );

      button.addEventListener(
        "pointerleave",
        () => {
          targetX = 0;
          targetY = 0;
        }
      );
    });

  /* ---------------------------------------------------------
     HERO 3D PARALLAX
  --------------------------------------------------------- */

  if (heroStage && !reduceMotion) {

    const floatingWidgets =
      Array.from(
        heroStage.querySelectorAll(
          "[data-float]"
        )
      );

    let pointerX = 0;
    let pointerY = 0;

    let smoothX = 0;
    let smoothY = 0;

    heroStage.addEventListener(
      "pointermove",
      (event) => {

        const rect =
          heroStage.getBoundingClientRect();

        pointerX =
          ((event.clientX - rect.left) /
            rect.width -
            0.5) *
          2;

        pointerY =
          ((event.clientY - rect.top) /
            rect.height -
            0.5) *
          2;
      }
    );

    heroStage.addEventListener(
      "pointerleave",
      () => {
        pointerX = 0;
        pointerY = 0;
      }
    );

    function animateHero() {

      smoothX +=
        (pointerX - smoothX) * 0.07;

      smoothY +=
        (pointerY - smoothY) * 0.07;

      floatingWidgets.forEach(
        (widget) => {

          const depth =
            Number(
              widget.dataset.float || 1
            );

          const x =
            smoothX * 12 * depth;

          const y =
            smoothY * -10 * depth;

          widget.style.setProperty(
            "--parallax-x",
            `${x}px`
          );

          widget.style.setProperty(
            "--parallax-y",
            `${y}px`
          );

          widget.style.transform =
            `translate3d(
              ${x}px,
              ${y}px,
              0
            )`;
        }
      );

      requestAnimationFrame(
        animateHero
      );
    }

    animateHero();
  }

  /* ---------------------------------------------------------
     HERO SCROLL MOVEMENT
  --------------------------------------------------------- */

  if (heroStage && !reduceMotion) {

    let ticking = false;

    function updateHeroOnScroll() {

      const rect =
        heroStage.getBoundingClientRect();

      const viewportCenter =
        window.innerHeight / 2;

      const elementCenter =
        rect.top + rect.height / 2;

      const distance =
        elementCenter - viewportCenter;

      const offset =
        Math.max(
          -30,
          Math.min(
            30,
            distance * -0.035
          )
        );

      heroStage.style.setProperty(
        "--scroll-shift",
        `${offset}px`
      );

      ticking = false;
    }

    window.addEventListener(
      "scroll",
      () => {

        if (ticking) return;

        requestAnimationFrame(
          updateHeroOnScroll
        );

        ticking = true;
      },
      { passive: true }
    );

    updateHeroOnScroll();
  }

  /* ---------------------------------------------------------
     WIDGET HOVER DEPTH
  --------------------------------------------------------- */

  const widgets =
    document.querySelectorAll(
      ".widget, .floating-widget, .tilt-card"
    );

  widgets.forEach((widget) => {

    if (reduceMotion) return;

    widget.addEventListener(
      "pointerenter",
      () => {

        widget.style.transition =
          "transform 500ms cubic-bezier(.22,1,.36,1), box-shadow 500ms cubic-bezier(.22,1,.36,1)";

        widget.style.zIndex = "20";

        widget.style.transform =
          "translateY(-8px) scale(1.025)";
      }
    );

    widget.addEventListener(
      "pointerleave",
      () => {

        widget.style.transform = "";

        widget.style.zIndex = "";
      }
    );
  });

  /* ---------------------------------------------------------
     WIDGET CLICK DEPTH
  --------------------------------------------------------- */

  document
    .querySelectorAll(
      ".widget, .floating-widget"
    )
    .forEach((widget) => {

      widget.addEventListener(
        "click",
        () => {

          if (reduceMotion) return;

          widget.animate(
            [
              {
                transform:
                  "scale(1)",
              },
              {
                transform:
                  "scale(.97)",
              },
              {
                transform:
                  "scale(1)",
              },
            ],
            {
              duration: 380,
              easing:
                "cubic-bezier(.22,1,.36,1)",
            }
          );
        }
      );
    });

  /* ---------------------------------------------------------
     LAYOUT SHUFFLE
  --------------------------------------------------------- */

  const shuffleButton =
    document.querySelector(
      "#shuffleLayout"
    );

  const layoutBoard =
    document.querySelector(
      ".layout-demo .layout-board"
    );

  if (
    shuffleButton &&
    layoutBoard
  ) {

    shuffleButton.addEventListener(
      "click",
      () => {

        const cards =
          Array.from(
            layoutBoard.children
          );

        if (cards.length < 2) return;

        const oldPositions =
          new Map();

        cards.forEach((card) => {
          oldPositions.set(
            card,
            card.getBoundingClientRect()
          );
        });

        const shuffled =
          [...cards].sort(
            () => Math.random() - 0.5
          );

        shuffled.forEach(
          (card) => {
            layoutBoard.appendChild(card);
          }
        );

        /*
          FLIP animation
        */

        shuffled.forEach((card) => {

          const newRect =
            card.getBoundingClientRect();

          const oldRect =
            oldPositions.get(card);

          if (!oldRect) return;

          const deltaX =
            oldRect.left -
            newRect.left;

          const deltaY =
            oldRect.top -
            newRect.top;

          const deltaScaleX =
            oldRect.width /
            newRect.width;

          const deltaScaleY =
            oldRect.height /
            newRect.height;

          card.animate(
            [
              {
                transform:
                  `translate(
                    ${deltaX}px,
                    ${deltaY}px
                  )
                  scale(
                    ${deltaScaleX},
                    ${deltaScaleY}
                  )`,
              },
              {
                transform:
                  "translate(0,0) scale(1)",
              },
            ],
            {
              duration: 700,
              easing:
                "cubic-bezier(.22,1,.36,1)",
            }
          );
        });
      }
    );
  }

  /* ---------------------------------------------------------
     SMOOTH ANCHOR SCROLL
  --------------------------------------------------------- */

  document
    .querySelectorAll(
      'a[href^="#"]'
    )
    .forEach((link) => {

      link.addEventListener(
        "click",
        (event) => {

          const id =
            link.getAttribute("href");

          if (
            !id ||
            id === "#"
          ) {
            return;
          }

          const target =
            document.querySelector(id);

          if (!target) return;

          event.preventDefault();

          target.scrollIntoView({
            behavior:
              reduceMotion
                ? "auto"
                : "smooth",
            block: "start",
          });
        }
      );
    });

  /* ---------------------------------------------------------
     IMAGE BOX HOVER
  --------------------------------------------------------- */

  document
    .querySelectorAll(
      ".image-box, .image-preview"
    )
    .forEach((imageBox) => {

      if (reduceMotion) return;

      imageBox.addEventListener(
        "pointermove",
        (event) => {

          const rect =
            imageBox.getBoundingClientRect();

          const px =
            (event.clientX -
              rect.left) /
            rect.width;

          const py =
            (event.clientY -
              rect.top) /
            rect.height;

          const moveX =
            (px - 0.5) * 8;

          const moveY =
            (py - 0.5) * -8;

          imageBox.style.setProperty(
            "--image-x",
            `${moveX}px`
          );

          imageBox.style.setProperty(
            "--image-y",
            `${moveY}px`
          );
        }
      );

      imageBox.addEventListener(
        "pointerleave",
        () => {

          imageBox.style.setProperty(
            "--image-x",
            "0px"
          );

          imageBox.style.setProperty(
            "--image-y",
            "0px"
          );
        }
      );
    });

  /* ---------------------------------------------------------
     BUTTON PRESS MICRO-INTERACTION
  --------------------------------------------------------- */

  document
    .querySelectorAll(
      "button, .button, .btn, a.cta"
    )
    .forEach((button) => {

      if (reduceMotion) return;

      button.addEventListener(
        "pointerdown",
        () => {

          button.animate(
            [
              {
                transform:
                  "scale(1)",
              },
              {
                transform:
                  "scale(.96)",
              },
              {
                transform:
                  "scale(1)",
              },
            ],
            {
              duration: 260,
              easing:
                "cubic-bezier(.22,1,.36,1)",
            }
          );
        }
      );
    });

  /* ---------------------------------------------------------
     ACTIVE NAVIGATION
  --------------------------------------------------------- */

  const sections =
    document.querySelectorAll(
      "section[id]"
    );

  const navLinks =
    document.querySelectorAll(
      'nav a[href^="#"]'
    );

  if (
    sections.length &&
    navLinks.length &&
    "IntersectionObserver" in window
  ) {

    const sectionObserver =
      new IntersectionObserver(
        (entries) => {

          entries.forEach(
            (entry) => {

              if (!entry.isIntersecting)
                return;

              const id =
                entry.target.id;

              navLinks.forEach(
                (link) => {

                  link.classList.toggle(
                    "active",
                    link.getAttribute(
                      "href"
                    ) === `#${id}`
                  );
                }
              );
            }
          );
        },
        {
          rootMargin:
            "-35% 0px -50% 0px",
          threshold: 0,
        }
      );

    sections.forEach(
      (section) =>
        sectionObserver.observe(section)
    );
  }

  /* ---------------------------------------------------------
     FLOATING DECORATION
  --------------------------------------------------------- */

  if (!reduceMotion) {

    const decorative =
      document.querySelectorAll(
        "[data-floating]"
      );

    decorative.forEach(
      (element, index) => {

        const duration =
          3500 + index * 600;

        const distance =
          8 + (index % 3) * 4;

        element.animate(
          [
            {
              transform:
                "translate3d(0,0,0)",
            },
            {
              transform:
                `translate3d(
                  0,
                  -${distance}px,
                  0
                )`,
            },
            {
              transform:
                "translate3d(0,0,0)",
            },
          ],
          {
            duration,
            iterations: Infinity,
            easing:
              "ease-in-out",
          }
        );
      }
    );
  }

  /* ---------------------------------------------------------
     PAGE LOAD
  --------------------------------------------------------- */

  document.body.classList.add(
    "mosaic-loaded"
  );

  window.setTimeout(() => {
    document.body.classList.add(
      "mosaic-ready"
    );
  }, 80);

  /* ---------------------------------------------------------
     CONSOLE BRANDING
  --------------------------------------------------------- */

  console.log(
    "%c MOSAIC ",
    "font-size:20px;font-weight:800;"
  );

  console.log(
    "Your desktop, arranged around you."
  );
});