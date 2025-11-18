# boxes-live Demo Video Processing

**Date:** 2025-11-17
**Video:** boxes-live-demo.webm (12MB)
**Processed:** boxes-live-demo-1.7x.mp4 (17MB)

## Processing Applied

```bash
process-demo-video /mnt/c/Users/JefferyCaldwell/Downloads/boxes-live-demo.webm
```

### Optimizations
- ✅ **Speed:** 1.7x (6+ min → 3:36 min)
- ✅ **Codec:** H.264 with CRF 23 (web-compatible)
- ✅ **Streaming:** moov atom at start (faststart)
- ✅ **Compression:** Balanced quality/size
- ✅ **Key Frames:** 5 PNGs extracted for documentation

## Output Files

**Video:**
- `boxes-live-demo-1.7x.mp4` - 17MB, 3:36 duration

**Screenshots:**
- `frame-001.png` (233KB) - Initial screen
- `frame-002.png` (123KB) - Early demo
- `frame-003.png` (256KB) - Mid demo
- `frame-004.png` (125KB) - Advanced features
- `frame-005.png` (181KB) - Final state

## Future Demo Videos

All future boxes-live demos should use the official workflow:

```bash
# Standard demo
process-demo-video demo.webm

# Detailed tutorial (slower)
process-demo-video -s 1.2 -q 20 -f 8 tutorial.webm

# Quick teaser (faster)
process-demo-video -s 2.5 -f 3 teaser.webm
```

See `~/bin/VIDEO-WORKFLOW.md` for complete documentation.

## Why 1.7x Speed?

Terminal demos benefit from faster playback:
- Shows functionality without tedium
- Viewers can pause if needed
- Maintains engagement
- 41% shorter → better retention

## Integration Points

These screenshots should be added to:
- [ ] Project README.md
- [ ] GitHub repository description
- [ ] Documentation site
- [ ] Social media posts
- [ ] Demo presentations

## JCaldwell Labs Standard

This processing is now the **official standard** for all demo videos:
- Location: `~/bin/process-demo-video`
- Documentation: `~/bin/VIDEO-WORKFLOW.md`
- Default settings: 1.7x speed, CRF 23, 5 key frames
