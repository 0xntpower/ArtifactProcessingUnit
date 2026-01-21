![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg) ![MSVC](https://img.shields.io/badge/compiler-MSVC-blue)

# ArtifactProcessingUnit

A secure, modular C++ system for receiving, validating, analyzing, and classifying digital artifacts at scale.

## Overview

**ArtifactProcessingUnit** is responsible for the end-to-end lifecycle of artifacts:

* Securely receiving artifacts over the network
* Assigning cryptographic identity and metadata
* Extracting and analyzing content
* Classifying artifacts by relevance and interest level
* Persisting state and analysis results in SQLite

The system is designed to be extensible, resilient to malformed input, and suitable for long-running automated document processing pipelines.

## Core Components

* **Artifacts Receiver**
  Receives artifacts over gRPC (TLS), decodes them, assigns a unique identity, and stores initial metadata.

* **Artifacts Manager**
  Periodically scans the artifacts directory, validates artifacts, handles retries, and isolates bad artifacts based on configurable policy.

* **Artifact Processor**
  Orchestrates content extraction and LLM-based analysis to determine format, language, description, and interest level.

* **Content Extractors**
  Pluggable extractors for different artifact formats (DOCX, PDF, media, etc.).

* **Content Processors**
  Modular LLM-driven processors for semantic analysis and classification.

## Tech Stack

* **Language:** C++ 20
* **Networking:** gRPC over TLS
* **Database:** SQLite
* **Language Detection:** Google CLD3
* **Integrity:** BLAKE3 hashing

## Execution

```bash
ArtifactsProcUnit.exe \
  --artifacts path/to/artifacts \
  --interesting path/to/interesting \
  --notinteresting path/to/notinteresting \
  --badartifacts path/to/badartifacts
```
```bash
ArtifactsProcUnit.exe --detectlocal
```

## Configuration

Behavior is controlled via JSON configuration files:

* Scan intervals and batch sizes
* Artifact size limits and retry thresholds
* Bad artifact handling strategy
* Optional VirusTotal integration
* LLM prompt and verdict baselines

## Design Goals

* Deterministic processing and traceability
* Strong artifact identity and integrity guarantees
* Fault-tolerant handling of corrupted or incomplete input
* Clean separation between extraction, analysis, and storage
* Easy extensibility for new artifact formats and processors