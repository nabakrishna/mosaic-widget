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
     LABU MASCOT
  --------------------------------------------------------- */

  const mascot =
    document.getElementById("labu-mascot");
  const mascotBubble =
    document.getElementById("labu-bubble");
  const mascotPupils =
    mascot?.querySelector(".labu-pupils");

  if (mascot && mascotBubble && mascotPupils) {
    const targetSelector =
      'a, button, input, textarea, select, summary, [role="button"], main > section, .site-header, .site-footer, .footer-brand, .platform-row, .feature, .orbit-card, .screenshot, .faq-item, .source-list > div, .stats > div, .testimonial-card, .imagebox-browser, .hero-copy, .story-copy, .performance-copy, .download-inner';
    const getHoverTarget = (target) => {
      if (!(target instanceof Element) || mascot.contains(target)) return null;
      return target.closest(targetSelector);
    };
    const mascotSize = () => {
      const rect = mascot.getBoundingClientRect();
      return { width: rect.width, height: rect.height };
    };
    function positionBubble() {
      if (mascotBubble.hidden) return;

      const mascotRect = mascot.getBoundingClientRect();
      const bubbleRect = mascotBubble.getBoundingClientRect();
      const preferredLeft = mascotRect.width / 2 - bubbleRect.width / 2;
      const minLeft = 8 - mascotRect.left;
      const maxLeft =
        window.innerWidth - bubbleRect.width - 8 - mascotRect.left;

      mascotBubble.style.left =
        `${Math.max(minLeft, Math.min(maxLeft, preferredLeft))}px`;
      mascotBubble.style.transform = "none";

      if (mascotRect.top - bubbleRect.height >= 8) {
        mascotBubble.style.top = "auto";
        mascotBubble.style.bottom = "100%";
      } else {
        mascotBubble.style.top = `${mascotRect.height + 8}px`;
        mascotBubble.style.bottom = "auto";
      }
    }
    const size = mascotSize();
    let x = Math.max(8, window.innerWidth - size.width - 16);
    let y = Math.max(72, window.innerHeight - size.height - 16);
    let targetX = x;
    let targetY = y;
    let pointerX = x;
    let pointerY = y;
    let mode = "idle";
    let currentTarget = null;
    let hasPointerPosition = false;
    let nextRunAt = 0;
    let moodTimer = 0;
    let idleTimer = 0;
    let clicks = [];
    let hoverRefreshFrame = 0;
    let rest = "";
    let restBy = "";
    let facing = false;
    let lastActivity = Date.now();
    let lastDizzy = 0;
    let cheers = 0;
    let away = 0;
    let awayTimers = [];
    let exitPoint = { x: 0, y: 0 };
    let sectionKey = "hero";
    const hour = new Date().getHours();
    const night = hour >= 22 || hour < 5;

    function showMascotMood(
      mood,
      message,
      movement,
      duration = 0,
      pose = "",
      running = false
    ) {
      window.clearTimeout(moodTimer);
      rest = "";
      mascot.dataset.m = mood;
      mascot.classList.toggle("run", running);
      mascot.classList.remove(
        "pose-curious",
        "pose-thinking",
        "pose-proud",
        "pose-dance",
        "pose-sit",
        "pose-float",
        "pose-lie",
        "pose-dizzy"
      );
      if (pose) mascot.classList.add(`pose-${pose}`);
      mode = movement;
      mascotBubble.textContent = message;
      mascotBubble.hidden = !message;
      positionBubble();
      if (reduceMotion) animateMascot();

      if (duration > 0) {
        moodTimer = window.setTimeout(() => {
          if (currentTarget?.isConnected) {
            placeBeside(currentTarget);
            showMascotMood(
              "point",
              getTargetMessage(currentTarget),
              "point"
            );
          } else {
            showMascotMood("happy", "", "idle");
          }
        }, duration);
      }
    }

    // sit / sleep until the visitor comes back
    function settle(kind, message) {
      currentTarget = null;
      clearAway();
      facing = false;
      showMascotMood(
        kind === "sleep" ? "sleep" : "happy",
        message,
        kind === "sleep" ? "float" : "idle",
        0,
        kind === "sleep" ? "float" : kind
      );
      rest = kind;
      restBy = "idle";
      if (message) {
        window.setTimeout(() => {
          if (rest === kind) mascotBubble.hidden = true;
        }, 2600);
      }
    }

    function wake(message) {
      lastActivity = Date.now();
      if (!rest) return;
      showMascotMood("laugh", message, "idle", 1600, "", !reduceMotion);
    }

    function clearAway() {
      awayTimers.forEach((id) => window.clearTimeout(id));
      awayTimers = [];
      away = 0;
    }

    // cursor left the window: look around, wait sadly, then lie down and cry
    function startAway(event) {
      if (away || document.hidden || event.relatedTarget) return;
      exitPoint = {
        x: Math.max(0, Math.min(window.innerWidth, event.clientX)),
        y: Math.max(0, Math.min(window.innerHeight, event.clientY))
      };
      currentTarget = null;
      showMascotMood("happy", "Where did you go? 🥺", reduceMotion ? "idle" : "run", 0, "", !reduceMotion);
      away = 1;
      awayTimers.push(window.setTimeout(() => {
        const { width, height } = mascotSize();
        targetX = exitPoint.x <= 0 ? 0 : exitPoint.x >= window.innerWidth ? window.innerWidth - width : exitPoint.x - width / 2;
        targetY = exitPoint.y <= 0 ? 56 : exitPoint.y >= window.innerHeight ? window.innerHeight - height : exitPoint.y - height / 2;
        showMascotMood("sad", "I'll wait right here… 😢", "stay", 0, "sit");
        away = 2;
      }, 2000));
      awayTimers.push(window.setTimeout(() => {
        showMascotMood("sad", "I miss you… please come back 😭", "stay", 0, "lie");
        away = 3;
      }, 6000));
    }

    function noteActivity() {
      lastActivity = Date.now();
      cheers = 0;
      if (away) {
        const crying = away === 3;
        clearAway();
        showMascotMood("laugh", crying ? "You came back! 🥹" : "There you are! 🥰", "idle", 2000, "", !reduceMotion);
        return;
      }
      if (rest === "sleep") wake("Oh! You're back! 👋");
      else if (rest === "sit") showMascotMood("happy", "", "idle");
    }

    function getTargetMessage(element) {
      if (element.dataset.labuTip) return element.dataset.labuTip;

      const label =
        element.getAttribute("aria-label") ||
        element.getAttribute("title") ||
        element.querySelector("h1, h2, h3, h4")?.textContent ||
        (element.matches("input, textarea")
          ? element.getAttribute("placeholder")
          : element.innerText || element.textContent);
      const normalizedLabel = label?.replace(/\s+/g, " ").trim();
      const cleanLabel = normalizedLabel?.length > 72
        ? `${normalizedLabel.slice(0, 69).trimEnd()}…`
        : normalizedLabel;

      if (element.matches("input, textarea, select")) {
        return cleanLabel
          ? `You can use this ${cleanLabel.toLowerCase()}! ✨`
          : "Try typing something here! ✨";
      }

      if (element.matches("a, button, [role='button']")) {
        return cleanLabel
          ? `Want to try ${cleanLabel}? 👆`
          : "Tap here to explore! 👆";
      }

      return cleanLabel
        ? `Here's ${cleanLabel}! ✨`
        : "Take a look at this part of Mosaic! ✨";
    }

    function reactToMascotClick() {
      const now = Date.now();
      lastActivity = now;
      if (rest === "sleep") {
        showMascotMood("laugh", "Hey, I was napping! 😝", "idle", 1800, "", !reduceMotion);
        return;
      }
      clicks = clicks.filter((time) => now - time < 1500);
      clicks.push(now);

      if (clicks.length >= 4) {
        clicks = [];
        showMascotMood("sad", "Ouch! That tickles… 😢", "idle", 2600);
      } else {
        showMascotMood("laugh", "Hehe! 😄", "idle", 900);
      }
    }

    function isOverMascot(x, y) {
      const rect = mascot.getBoundingClientRect();
      return x >= rect.left && x <= rect.right &&
        y >= rect.top && y <= rect.bottom;
    }

    function placeBeside(element) {
      const rect = element.getBoundingClientRect();
      const { width, height } = mascotSize();
      const left = rect.left > width + 16
        ? rect.left - width + 18
        : Math.min(window.innerWidth - width, rect.right + 12);

      facing = !(rect.left > width + 16);
      targetX = left;
      targetY = rect.top + rect.height / 2 - height / 2;
    }

    function updateHoverTarget(element) {
      if (element === currentTarget) return;
      currentTarget = element;

      if (!element) {
        if (mode === "point") showMascotMood("happy", "", "idle");
        return;
      }

      placeBeside(element);
      showMascotMood("point", getTargetMessage(element), "point");
    }

    function refreshHoverTarget() {
      if (!hasPointerPosition || hoverRefreshFrame) return;

      hoverRefreshFrame = window.requestAnimationFrame(() => {
        hoverRefreshFrame = 0;
        updateHoverTarget(
          getHoverTarget(document.elementFromPoint(pointerX, pointerY))
        );
      });
    }

    function animateMascot() {
      const { width, height } = mascotSize();

      if (mode === "follow") {
        targetX = pointerX + 26;
        targetY = pointerY + 22;
      } else if (mode === "float") {
        const t = performance.now() / 1000;
        targetX = (window.innerWidth - width) / 2 + Math.sin(t * 0.35) * (window.innerWidth - width) * 0.38;
        targetY = window.innerHeight * 0.5 - height / 2 + Math.sin(t * 0.9) * 22;
      } else if (mode === "run" && performance.now() >= nextRunAt) {
        targetX = Math.random() * Math.max(1, window.innerWidth - width);
        targetY = 64 + Math.random() * Math.max(1, window.innerHeight - height - 64);
        nextRunAt = performance.now() + 700;
      }

      targetX = Math.max(0, Math.min(window.innerWidth - width, targetX));
      targetY = Math.max(48, Math.min(window.innerHeight - height, targetY));
      if (reduceMotion) {
        x = targetX;
        y = targetY;
      } else {
        x += (targetX - x) * (mode === "run" ? 0.13 : 0.06);
        y += (targetY - y) * 0.06;
      }
      const dx = targetX - x;
      if (mode !== "point" && mode !== "float" && Math.abs(dx) > 3) facing = dx < 0;
      mascot.classList.toggle("flip", facing);
      mascot.style.transform = `translate3d(${x}px, ${y}px, 0)`;
      positionBubble();

      if (!reduceMotion) {
        window.requestAnimationFrame(animateMascot);
      }
    }

    function scheduleIdleMoment() {
      window.clearTimeout(idleTimer);
      idleTimer = window.setTimeout(() => {
        if (mode === "idle" && !currentTarget && !rest && !away) {
          const moments = [
            { mood: "wave", pose: "", message: "Hey, nice to see you! 👋" },
            { mood: "happy", pose: "curious", message: "What shall we explore? ✨" },
            { mood: "happy", pose: "thinking", message: "" },
            { mood: "laugh", pose: "", message: "Having a happy little day! 😄" },
            { mood: "happy", pose: "proud", message: "" },
            { mood: "happy", pose: "dance", message: "A tiny dance break! 🎵" },
            { mood: "laugh", pose: "", message: "Catch me! 😆", movement: "run", running: true },
          ];
          const moment = moments[Math.floor(Math.random() * moments.length)];
          showMascotMood(
            moment.mood,
            moment.message,
            reduceMotion ? "idle" : moment.movement || "idle",
            2400,
            moment.pose,
            !reduceMotion && Boolean(moment.running)
          );

          if (moment.message && !reduceMotion) {
            targetX = Math.random() * Math.max(1, window.innerWidth - size.width);
            targetY = Math.random() * Math.max(1, window.innerHeight - size.height);
          }
        }
        scheduleIdleMoment();
      }, 3800 + Math.random() * 2600);
    }

    showMascotMood("wave", night ? "Working late? 🌙 I'm Labu 👋" : "Hi! I'm Labu 👋", "stay", 4200);
    scheduleIdleMoment();
    animateMascot();

    window.addEventListener("pointermove", (event) => {
      if (event.pointerType === "touch") return;
      pointerX = event.clientX;
      pointerY = event.clientY;
      hasPointerPosition = true;

      const rect = mascot.getBoundingClientRect();
      const angle = Math.atan2(
        pointerY - (rect.top + rect.height * 0.46),
        pointerX - (rect.left + rect.width / 2)
      );
      mascotPupils.style.transform =
        `translate(${Math.cos(angle) * 2.6}px, ${Math.sin(angle) * 2}px)`;
    }, { passive: true });
    window.addEventListener("scroll", refreshHoverTarget, { passive: true });

    document.addEventListener("pointerdown", (event) => {
      if (!isOverMascot(event.clientX, event.clientY)) return;
      event.preventDefault();
      event.stopImmediatePropagation();
      reactToMascotClick();
    }, true);

    document.addEventListener("pointerover", (event) => {
      if (event.pointerType === "touch") return;
      updateHoverTarget(getHoverTarget(event.target));
    });

    document.addEventListener("pointerout", (event) => {
      if (event.pointerType === "touch") return;
      const from = getHoverTarget(event.target);
      const to = getHoverTarget(event.relatedTarget);

      if (from && from !== to && from === currentTarget) {
        updateHoverTarget(to);
      }
    });

    document.addEventListener("focusin", (event) => {
      updateHoverTarget(getHoverTarget(event.target));
    });

    document.addEventListener("focusout", (event) => {
      const from = getHoverTarget(event.target);
      const to = getHoverTarget(event.relatedTarget);

      if (from && from !== to && from === currentTarget) {
        updateHoverTarget(to);
      }
    });

    document.addEventListener("click", (event) => {
      if (!(event.target instanceof Element)) return;
      if (event.detail > 0 && isOverMascot(event.clientX, event.clientY)) {
        event.preventDefault();
        event.stopImmediatePropagation();
        return;
      }

      const element = event.target.closest("[data-labu-say]");
      if (element) {
        showMascotMood("laugh", element.dataset.labuSay, "idle", 1800);
      }
    }, true);

    mascot.addEventListener("click", reactToMascotClick);

    mascot.addEventListener("focus", () => {
      mascotBubble.textContent = "Hi! I'm Labu. Press Enter to say hello! 👋";
      mascotBubble.hidden = false;
    });

    mascot.addEventListener("blur", () => {
      mascotBubble.hidden = true;
    });

    window.addEventListener("resize", () => {
      const { width, height } = mascotSize();
      targetX = Math.min(targetX, window.innerWidth - width);
      targetY = Math.min(targetY, window.innerHeight - height);
      if (reduceMotion) animateMascot();
    });

    /* ---- Labu: rest, sleep and page-aware moods ---- */
    ["pointermove", "pointerdown", "keydown", "wheel", "scroll", "touchstart"].forEach((type) =>
      window.addEventListener(type, noteActivity, { passive: true, capture: true })
    );

    const IDLE = {
      performance: ["sleep", "Quiet by design… 😴"],
      faq: ["sit", "Take your time with the FAQ 📖"],
      screenshots: ["sit", "Grab a seat and watch 🍿"],
      footer: ["sit", "Thanks for visiting! 💖"],
      download: ["cheer", "Ready when you are! 🎉"]
    };
    const ENTER = {
      intro: ["happy", "Here's the story ✨", "thinking"],
      features: ["happy", "So many widgets! ✨", "curious"],
      showcase: ["happy", "Ooh, pretty pictures! 🖼️", "curious"],
      download: ["laugh", "Ready to download? 🎉", "", true]
    };
    const sectionOf = (el) =>
      el.id || (el.classList.contains("hero") ? "hero" : el.classList.contains("intro") ? "intro" : "footer");

    if ("IntersectionObserver" in window) {
      const io = new IntersectionObserver((entries) => {
        entries.forEach((entry) => {
          if (!entry.isIntersecting) return;
          sectionKey = sectionOf(entry.target);
          const m = ENTER[sectionKey];
          if (m && mode === "idle" && !currentTarget && !rest) {
            showMascotMood(m[0], m[1], "idle", 2400, m[2], Boolean(m[3]) && !reduceMotion);
          }
        });
      }, { rootMargin: "-40% 0px -40% 0px" });
      document.querySelectorAll("main > section, .site-footer").forEach((el) => io.observe(el));
    }

    window.setInterval(() => {
      if (document.hidden || away) return;
      const idle = Date.now() - lastActivity;
      const k = night ? 0.7 : 1;
      const act = IDLE[sectionKey];

      if (!rest && (mode === "idle" || mode === "point") && idle > 3000 * k) {
        if (act?.[0] === "cheer" && cheers++ < 3) {
          lastActivity = Date.now() - 1000;
          showMascotMood("laugh", act[1], "idle", 2400, "dance");
        } else if (act?.[0] === "sleep") settle("sleep", act[1]);
        else settle("sit", act?.[1] || "Taking a little break 🪑");
      } else if (rest === "sit" && idle > 7000 * k) {
        settle("sleep", "Zzz… 😴");
      } else if (!rest && idle > 12000) {
        settle("sleep", "Zzz… 😴");
      }
    }, 1000);

    document.addEventListener("visibilitychange", () => {
      if (document.hidden) settle("sleep", "");
      else wake("Welcome back! 👋");
    });
    document.documentElement.addEventListener("mouseleave", startAway);
    document.documentElement.addEventListener("mouseenter", noteActivity);
    window.addEventListener("offline", () => showMascotMood("sad", "No internet… 😢", "idle", 3000));
    window.addEventListener("online", () => showMascotMood("laugh", "We're back online! 🎉", "idle", 2200));

    let lastScrollY = window.scrollY;
    let lastScrollAt = performance.now();
    window.addEventListener("scroll", () => {
      const t = performance.now();
      const speed = Math.abs(window.scrollY - lastScrollY) / Math.max(1, t - lastScrollAt);
      lastScrollY = window.scrollY;
      lastScrollAt = t;
      if (speed > 3 && t - lastDizzy > 7000 && !reduceMotion) {
        lastDizzy = t;
        showMascotMood("happy", "Whoa, so fast! 😵", "idle", 1500, "dizzy");
      }
    }, { passive: true });
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