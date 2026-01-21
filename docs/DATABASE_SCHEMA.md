# Database Schema

SQLite database for artifact metadata storage and full-text search.

## Tables

### artifacts

Primary table storing artifact metadata and processing state.

| Column | Type | Description |
|--------|------|-------------|
| id | INTEGER | Primary key |
| unique_id | TEXT | UUID, unique, not null |
| blake3_signature | TEXT | BLAKE3 hash of artifact |
| original_name | TEXT | Original filename |
| state | TEXT | RECEIVED, PROCESSING, PROCESSED, BAD |
| attempts | INTEGER | Processing attempt count |
| operation | TEXT | Source operation name |
| machine | TEXT | Source machine identifier |
| ip | TEXT | Source IP address |
| target_name | TEXT | Target identifier |
| format | TEXT | Detected file format |
| language | TEXT | Detected content language |
| description | TEXT | LLM-generated description |
| interest_level | TEXT | NotInteresting, Interesting, Critical |
| extracted_size_bytes | INTEGER | Size of extracted content |
| created_at | DATETIME | Record creation timestamp |
| processed_at | DATETIME | Processing completion timestamp |

### extracted_content

FTS5 virtual table for full-text search on extracted artifact content.

| Column | Type | Description |
|--------|------|-------------|
| artifact_id | TEXT | Foreign key to artifacts |
| content | TEXT | Extracted text content |

## Indexes

| Index | Column | Purpose |
|-------|--------|---------|
| idx_interest_level | interest_level | Filter by classification |
| idx_state | state | Query processing queue |

## Notes

- FTS5 tokenizer: `porter unicode61` for stemming and unicode support
- State constraint enforced via CHECK clause
- Interest level constraint enforced via CHECK clause
