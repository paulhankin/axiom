## Fixes

- The optimizer (-O1 and up) could crash on a loop of one block when an operation feeding
  one of the block's phis was computed in that same block: it tried to move the operation
  to the block's start. Such operations now stay where they are.
