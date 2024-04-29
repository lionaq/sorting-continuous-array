
# Sorting Contiguous Array 
_(exchange at edges) (elements streamed to any of the participating nodes)_

**INITIAL STATE:**
  - processors contain a slice of unsorted array
  - there are no slaves/master

 
**PROCESSING:**
  - processors locally sort their elements, then exchange the items with other processors
  - during exchange, send one element at a time. **_Do not exchange the entire array_**.

 
**FINAL STATE:**
  - the elements in the array are arranged in increasing order from processor **_0 to n_**
