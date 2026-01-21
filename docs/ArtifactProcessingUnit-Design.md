# The Dopamine Project
## Component - ArtifactProcessingUnit

### Techstack
- **Programming Language:** `C++`
- **Network Communication:** `gRPC over TLS`
- **Database:** `SQLite`

#### Other
- **Language-Detection:** `Google's C++ CLD3 library`

#### Execution Arguments
```
ArtifactsProcUnit.exe --artifacts "path/to/artifacts" --interesting "path/to/interesting" --notinteresting "path/to/notinteresting" --badartifacts "path/to/badartifacts"
```

## Artifacts Receiver

Responsible for receiving new artifacts over the network, decoding the artifacts in whatever way needed (decompressing / decrypting) and creating a new ArtifactMedata for the new artifact in the SQLite database and filling all the source and and identity fields, also renaming the artifact to a newly generated UniqueId for it and saving it into the artifacts directory.

## Artifacts Manager

Periodically scans for new artifacts in the artifacts folder.

Config.json
```json
{
    "scan-internal": "3m",
    "processing-batch-size": 20,
    "max-artifact-size": "10gb",
    "max-processing-attempts": 100,
    "bad-artifact-handling": "DELETE/STORE",
    "VirusTotal":
    {
        "use-vt": false,
        "vendors-threshold": 60
    }
}
```

It will look for artifacts up to the configurable processing-batch-size value, if there are more it will process the next documents in the next batch.

for every artifact it finds in the folder it will check if following conditions first to determine if its valid for processing:
- is it still being received over the network.
- is it corrupted
- is it equal or below `max-artifact-size` value
- (if VirusTotal enabled) is the `vendors-threshold` amount of VT vendors flag it as malicious

if the file is found to be not valid for processing `max-processing-attempts` times its defined is a bad artifact and will be either deleted or stored in the badartifacts directory (determined by the `bad-artifact-handling` config value).

If the DELETE option was selected at `bad-artifact-handling`, the metadata is also deleted from the SQLite database. however if its configured to store it the file will be moved to the badartifacts directory and the metadata is kept and the `is-bad-artifact` metadata field will be set to true.

#### ArtifactMedata
```json
{
    "Processing": {
        "State": "RECEIVED/PROCESSED",
        "attempts": 0,
        "is-bad-artifact": false
    },
    "Source":
    {
        "Operation": "Sorting out my private documents server",
        "Machine": "Windows 11 Pro For Workstations",
        "IP": "192.168.0.0",
        "TargetName": "whoami"
    },
    "Identity":
    {
        "UniqueId": "9d9c0e11-5690-49af-8df1-5103d196d0d3",
        "BLAKE3-Signature": "fb4d4ea09a1f88ffc850329ce1155a623accc9c686e1d68ee8e3f44088762581",
        "Original_Name": "Four year plan"
    },
    "Analysis":
    {
        "Format": "DOCX/PDF/MP4/...",
        "Language": "English/Hebrew/Arabic/....",
        "Description": "2 - 4 line description of the contents and purpose of the artifact",
        "Interest-Level": "NotInteresting / Interesting / Critical"
    },
    "Extracted":
    {
        "Size": "Size of extracted content in KB/MB/GB/TB/...",
        "Content": "The contents that were extracted from the article"
    }
}
```

## Artifact Processor

A modular system, that uses different artifact content extractors and processors to figure out the ownership and content subject of the document and assess whether its interesting or not. 

The processor returns an updated version of the ArtifactMedata object with all the "Analysis" fields set.

#### Artifact Content Extractor

A modular system, that implements different classes for the required logic for extracting the content of different artifact types.

#### Artifact Content Processor

A modular system, that implements different classes for processing the different kinds of artifacts contents using an LLM.

**Configs:**
- LLMContentConfig.json

```json
{
    "BASE-PROMPT": "You are a professional data analyist...",
    "VERDICT-BASELINE": "A document is critical if it contains information relating to.... its interesting if
    it contains information relating to.... otherwise its not interesting."
}
```