---
layout: page
title: Character Encoding
parent: More Details
nav_order: 150
---

- TOC
{:toc}

Forge treats all internal strings as opaque UTF-8 byte sequences.  This applies to target identifiers, paths, branches, filenames, environment values, and anything else that flows between Lua, the C++ core, and the operating system.

Paths from operating system APIs are treated as UTF-8.  On Linux and macOS the filesystem and process APIs are UTF-8 by default.  On Windows, Forge declares a UTF-8 active code page in its application manifest so that the narrow `*A` Win32 and C runtime APIs round-trip UTF-8 bytes.  Windows 10 version 1903 or later is required because of this.

## Normalization

Forge does not perform Unicode normalization.  Strings are compared as byte sequences.  Two strings that look identical on screen but use different Unicode normalization forms (for example NFC `café` versus NFD `café`) are not equal to Forge.

In practice this means that paths originating from different sources must use the same byte representation to match.  If a target is authored in a buildfile and the same path is later reported by a filesystem hook, both must agree byte-for-byte.

This is deliberate to keep Forge simple.  It's the authors responsibility to keep identifiers in build scripts and paths consistent.

## Validation

Forge does no validatation that strings are well-formed UTF-8.  Invalid byte sequences will pass through unchanged and may produce unreadable output when printed, but they will not crash the build or corrupt the build cache.
