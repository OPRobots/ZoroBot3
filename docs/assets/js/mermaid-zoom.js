/* ==========================================================================
   Mermaid diagram zoom — lightbox overlay on click
   ========================================================================== */

(function () {
  "use strict";

  var shadowMap = window.__mermaidShadowMap;
  if (!shadowMap) {
    console.warn(
      "[Mermaid Zoom] attachShadow interception not found. " +
        "Ensure the <script> in extrahead block runs before the Material bundle."
    );
    return;
  }

  var activeOverlay = null;
  var cleanupFns = [];

  // ------------------------------------------------------------------
  //  Helpers
  // ------------------------------------------------------------------

  /** Retrieve and clone the SVG from a mermaid element's closed shadow root */
  function getSVGFromMermaid(el) {
    var shadow = shadowMap.get(el);
    if (!shadow) return null;
    var svg = shadow.querySelector("svg");
    if (!svg) return null;

    // Deep-clone and strip explicit dimensions so CSS controls scaling
    var clone = svg.cloneNode(true);
    clone.removeAttribute("height");
    clone.removeAttribute("width");
    clone.style.removeProperty("max-width");
    clone.style.removeProperty("width");
    clone.style.removeProperty("height");
    return clone;
  }

  // ------------------------------------------------------------------
  //  Lightbox overlay
  // ------------------------------------------------------------------

  function closeOverlay() {
    if (!activeOverlay) return;
    document.body.classList.remove("mermaid-overlay-open");
    activeOverlay.removeEventListener("click", onOverlayClick);
    document.removeEventListener("keydown", onKeyDown);
    cleanupFns.forEach(function (fn) { fn(); });
    cleanupFns = [];
    activeOverlay.remove();
    activeOverlay = null;
  }

  function onOverlayClick(e) {
    if (e.target === activeOverlay) closeOverlay();
  }

  function onKeyDown(e) {
    if (e.key === "Escape") closeOverlay();
  }

  function createOverlay(svgClone) {
    // Remove any existing overlay first
    if (activeOverlay) closeOverlay();

    // ---- Zoom / pan state ----
    var scale = 1;
    // layoutX/Y = SVG's CSS-layout top-left inside container (flex-centered, fixed)
    // panX/Y   = user drag offset (0 at rest)
    var svgLayoutX = 0;
    var svgLayoutY = 0;
    var panX = 0;
    var panY = 0;
    var baseWidth = null;
    var isPanning = false;
    var panStart = { x: 0, y: 0 };
    var panStartPan = { x: 0, y: 0 };

    // ---- Build DOM ----
    var overlay = document.createElement("div");
    overlay.className = "mermaid-overlay";

    var svgContainer = document.createElement("div");
    svgContainer.className = "mermaid-overlay-content";
    svgContainer.appendChild(svgClone);

    var closeBtn = document.createElement("button");
    closeBtn.className = "mermaid-overlay-close";
    closeBtn.innerHTML = "×"; // multiplication sign (×)
    closeBtn.setAttribute("aria-label", "Cerrar diagrama");
    closeBtn.addEventListener("click", closeOverlay);

    overlay.appendChild(svgContainer);
    overlay.appendChild(closeBtn);
    document.body.appendChild(overlay);

    activeOverlay = overlay;
    overlay.addEventListener("click", onOverlayClick);
    document.addEventListener("keydown", onKeyDown);
    document.body.classList.add("mermaid-overlay-open");

    // ---- Size & center SVG inside container ----
    void svgContainer.offsetHeight; // force layout
    var containerRect = svgContainer.getBoundingClientRect();
    var pad = 24; // matches CSS padding
    var contW = containerRect.width - pad * 2;
    var contH = containerRect.height - pad * 2;
    var vb = svgClone.viewBox.baseVal;
    var aspect = vb.width / vb.height;

    // Fit SVG within content area, maintaining viewBox aspect ratio
    if (contW / contH > aspect) {
      baseWidth = contH * aspect;
    } else {
      baseWidth = contW;
    }
    svgClone.style.width = baseWidth + "px";
    svgClone.style.height = "auto";

    // Center: padding + (content - svg) / 2
    svgLayoutX = pad + (contW - baseWidth) / 2;
    svgLayoutY = pad + (contH - baseWidth / aspect) / 2;
    panX = svgLayoutX; // translate offset from container border-box
    panY = svgLayoutY;

    // ---- Apply sizing + translate (no CSS scale → always sharp) ----
    function applyTransform() {
      // Clamp pan so at least MARGIN px of SVG remain visible in container
      var svgW = baseWidth * scale;
      var svgH = (baseWidth / aspect) * scale;
      var MARGIN = 80;

      var minX, maxX, minY, maxY;
      if (svgW > contW) {
        minX = pad + MARGIN - svgW;
        maxX = pad + contW - MARGIN;
      } else {
        minX = pad - svgW + MARGIN;
        maxX = pad + contW - MARGIN;
      }
      if (svgH > contH) {
        minY = pad + MARGIN - svgH;
        maxY = pad + contH - MARGIN;
      } else {
        minY = pad - svgH + MARGIN;
        maxY = pad + contH - MARGIN;
      }
      panX = Math.max(minX, Math.min(maxX, panX));
      panY = Math.max(minY, Math.min(maxY, panY));

      svgClone.style.width = svgW + "px";
      svgClone.style.height = "auto";
      svgClone.style.transform =
        "translate(" + panX + "px, " + panY + "px)";
      svgContainer.style.cursor = scale > 1 ? "grab" : "default";
    }

    applyTransform();

    // ---- Wheel → zoom (centered on cursor) ----
    function onWheel(e) {
      e.preventDefault();
      var delta = e.deltaY > 0 ? -0.15 : 0.15;
      var newScale = Math.max(1, Math.min(8, scale + delta));

      // Cursor relative to container border-box (matches panX/Y coords)
      var rect = svgContainer.getBoundingClientRect();
      var cx = e.clientX - rect.left;
      var cy = e.clientY - rect.top;

      var ratio = newScale / scale;
      panX = cx - ratio * (cx - panX);
      panY = cy - ratio * (cy - panY);
      scale = newScale;

      applyTransform();
    }

    // ---- Mouse drag → pan ----
    function onMouseDown(e) {
      if (e.button !== 0) return; // left button only
      e.preventDefault();
      isPanning = true;
      panStart.x = e.clientX;
      panStart.y = e.clientY;
      panStartPan.x = panX;
      panStartPan.y = panY;
      svgContainer.style.cursor = "grabbing";
    }

    function onMouseMove(e) {
      if (!isPanning) return;
      panX = panStartPan.x + (e.clientX - panStart.x);
      panY = panStartPan.y + (e.clientY - panStart.y);
      applyTransform();
    }

    function onMouseUp() {
      if (!isPanning) return;
      isPanning = false;
      svgContainer.style.cursor = scale > 1 ? "grab" : "default";
    }

    // ---- Double-click → reset zoom ----
    function onDblClick(e) {
      e.preventDefault();
      scale = 1;
      panX = svgLayoutX;
      panY = svgLayoutY;
      applyTransform();
    }

    // ---- Attach listeners ----
    svgContainer.addEventListener("wheel", onWheel, { passive: false });
    svgContainer.addEventListener("mousedown", onMouseDown);
    svgContainer.addEventListener("dblclick", onDblClick);
    document.addEventListener("mousemove", onMouseMove);
    document.addEventListener("mouseup", onMouseUp);

    // ---- Cleanup on close ----
    cleanupFns.push(function () {
      document.removeEventListener("mousemove", onMouseMove);
      document.removeEventListener("mouseup", onMouseUp);
    });

    // Trigger layout so the transition fires
    void overlay.offsetHeight;
    overlay.classList.add("mermaid-overlay--active");
  }

  // ------------------------------------------------------------------
  //  Mermaid element setup
  // ------------------------------------------------------------------

  function setupMermaidElement(el) {
    // Guard: already wrapped
    if (
      el.parentNode &&
      el.parentNode.classList.contains("mermaid-zoom-container")
    ) {
      return;
    }

    // Create wrapper
    var container = document.createElement("div");
    container.className = "mermaid-zoom-container";

    // Create zoom button
    var btn = document.createElement("button");
    btn.className = "mermaid-zoom-btn";
    btn.setAttribute("aria-label", "Ampliar diagrama");
    btn.setAttribute("title", "Ampliar diagrama");

    // Insert wrapper before the mermaid div, then move div inside
    el.parentNode.insertBefore(container, el);
    container.appendChild(el);
    container.appendChild(btn);

    // Click handler
    btn.addEventListener("click", function (e) {
      e.stopPropagation();
      var svgClone = getSVGFromMermaid(el);
      if (svgClone) {
        createOverlay(svgClone);
      } else {
        console.warn("[Mermaid Zoom] Could not retrieve SVG from shadow root");
      }
    });
  }

  // ------------------------------------------------------------------
  //  Process existing + future mermaid elements
  // ------------------------------------------------------------------

  function init() {
    // Process any <div class="mermaid"> already in the DOM
    var existing = document.querySelectorAll("div.mermaid");
    for (var i = 0; i < existing.length; i++) {
      setupMermaidElement(existing[i]);
    }

    // Watch for dynamically rendered mermaid elements
    var observer = new MutationObserver(function (mutations) {
      for (var m = 0; m < mutations.length; m++) {
        var addedNodes = mutations[m].addedNodes;
        for (var n = 0; n < addedNodes.length; n++) {
          var node = addedNodes[n];
          if (node.nodeType !== 1) continue; // skip text/comment nodes

          // Direct match
          if (node.classList && node.classList.contains("mermaid")) {
            setupMermaidElement(node);
          }
          // Check descendants (for cases where elements are nested)
          if (node.querySelectorAll) {
            var nested = node.querySelectorAll("div.mermaid");
            for (var d = 0; d < nested.length; d++) {
              setupMermaidElement(nested[d]);
            }
          }
        }
      }
    });

    observer.observe(document.body, {
      childList: true,
      subtree: true,
    });
  }

  // Ensure DOM is ready before observing
  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", init);
  } else {
    init();
  }
})();
