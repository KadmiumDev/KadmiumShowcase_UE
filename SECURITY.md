# Security Policy

For online documentation and policies, visit [kadmium.dev/security](https://www.kadmium.dev/security).

---

## 1. Reporting a Vulnerability

If you discover a potential security vulnerability in any Kadmium utility, framework, or web endpoint, please report it directly to us rather than opening a public issue.

* **Email:** `security@kadmium.dev`
* **Details to Include:** Proof-of-concept steps, affected binary/build version, and environment conditions.
* **Coordination:** We ask that you give us reasonable time to investigate and remediate the issue prior to public disclosure.

---

## 2. Executable Safety & SmartScreen Policy

Our desktop clients and standalone utilities (such as *Context Packer* and *Kadmium Launcher*) are distributed as independent software without commercial EV code-signing certificates.

* **Zero Malicious Behavior:** Binaries contain zero third-party telemetry scrapers, bundled adware, or background miners.
* **Inspectable Source:** Shared engines and solutions are available with inspectable source code under developer tiers for direct auditing.

---

## 3. Data Privacy & Local Execution

* **Offline-First Processing:** Tools operate entirely on your local machine and make zero outbound network requests with your codebase.
* **Workstation Path Sanitization:** Native routines automatically sanitize local paths (replacing user account paths with `[REDACTED]`) to prevent private username exposure in AI prompts.
* **Minimal Telemetry:** Any optional crash diagnostic telemetry is fully anonymized and contains no file contents, tokens, or credentials.
