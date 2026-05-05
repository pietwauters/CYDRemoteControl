# Contributing to CYDRemoteControl

Thank you for your interest in contributing to the OpenPiste fencing electronics platform. This is a volunteer-driven open source project, and contributions of all kinds are welcome — bug reports, fixes, new features, documentation improvements, and hardware/enclosure refinements.

---

## Before you start

Please read this document in full before opening a pull request. It covers the contribution workflow, code style expectations, and the Contributor License Agreement (CLA).

---

## How to contribute

### Reporting bugs

Open an issue on GitHub. Please include:

- A clear description of the problem
- Steps to reproduce it
- Your hardware (CYD board variant, battery, button)
- Your environment (PlatformIO version, ESP32 SDK version)
- Any relevant log output

### Suggesting features

Open an issue labelled `enhancement`. Describe what you would like to achieve and why it is useful for fencing officials, armourers, or the maker community. Proposals that align with the platform's goal — affordable, reliable, foolproof electronics for clubs and federations — are most likely to be accepted.

### Submitting a pull request

1. Fork the repository and create a branch from `master`.
2. Make your changes. Keep each pull request focused on a single concern.
3. Test on real hardware before submitting. The CYD Remote Control is a physical device; untested PRs are unlikely to be merged.
4. Update documentation (README, inline comments) if your change affects behaviour or configuration.
5. Open the pull request with a clear description of what changed and why.

---

## Code style

- Language: **C/C++** (PlatformIO / ESP-IDF)
- Follow the style of the surrounding code — consistency matters more than any particular convention
- Comment non-obvious logic, especially anything related to MQTT message handling or the touchscreen driver
- Avoid introducing new library dependencies without discussion in an issue first

---

## Contributor License Agreement (CLA)

To keep the project legally clean and ensure it can remain open source for the benefit of the fencing community, all contributors must agree to the following CLA.

**By submitting a pull request to this repository, you confirm that:**

1. **You wrote the contribution yourself**, or you have the right to submit it under the terms below.
2. **You grant the project maintainer (Piet Wauters) a perpetual, worldwide, royalty-free, irrevocable licence** to use, reproduce, modify, distribute, and sublicense your contribution as part of this project or any successor project, under the MIT License or any compatible open source licence.
3. **You retain copyright in your contribution.** The CLA is a licence grant, not a copyright transfer.
4. **You understand that your contribution will be made public** and distributed under the MIT License.
5. **If you are contributing on behalf of an employer or organisation**, you confirm that you have the authority to grant the above licence on their behalf.

This CLA is lightweight and modelled on common open source practice (Apache ICLA, Google CLA). It exists to protect contributors, users, and the long-term health of the project — not to create bureaucracy.

**You do not need to sign anything.** Submitting a pull request constitutes your agreement to these terms.

---

## Questions

Open an issue or reach out via the contact details on the [GitHub profile](https://github.com/pietwauters).
