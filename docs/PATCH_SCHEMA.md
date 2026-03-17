# Patch Schema v1

```json
{
  "schemaVersion": 1,
  "nodes": [
    { "id": "...", "type": 1, "x": 120.0, "y": 220.0, "gain": 1.0 }
  ],
  "edges": [
    { "id": "...", "fromPortId": "...", "toPortId": "..." }
  ]
}
```

## Notes

- `type` maps to `NodeType` enum order.
- Ports are currently implicit by node type.
- Future version should explicitly persist port metadata.
