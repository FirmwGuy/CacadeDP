Cascade Data Processing
=======================

**Cascade Data Processing (CDP)** is a distributed execution solution designed 
to run on a wide range of platforms, from powerful servers down to 
microcontrollers and embedded systems. CDP is open source, released under the 
MIT license.

> **Note:**
> This project is under active development, currently in an experimental state
and **not yet ready for production use**!

---


Overview
--------

CDP is inspired by the Plan 9 philosophy—**everything is represented as a 
virtual record system**. The core of CDP is a highly flexible, distributed data 
and processing framework based on "records" (data units) and "agents" 
(processing units). This abstraction allows CDP to unify data storage and 
computation under a modular umbrella.

**Key Features:**

* Flexible and dynamic data storage structures, selectable at runtime.

* Distributed and decentralized—no central authority required.

* Modular, agent-based execution pipeline.

* Designed for portability, including embedded and resource-constrained 
environments.

These features aim to make CDP scalable and adaptable to diverse application 
domains.

---


Core Concepts
-------------

To understand CDP's architecture, it is necessary to grasp its fundamental 
concepts. These include Records, the data containers; Agents and Agencies, the 
processing units; and Tasks, the vehicles of computation. The following 
sections describe these in detail.

### Records

* **Definition:**
  Records are the fundamental data unit in CDP. A record can contain binary 
  data, metadata, and may also contain other records as children. This 
  recursive structure enables complex, hierarchical data models.

* **Naming:**
  Every record has a name (text or number), enabling dictionary-like storage 
  and natural tree hierarchies. The names are bi-dimensional, having two parts: 
  a "domain" and a "tag" (DT).

* **Naming & Hierarchy:**
  The user (or the CDP system) can assign names. Those DT may be used to query 
  a record about selected children with the same domain, tag, or both. This 
  naming system supports straightforward data navigation and organization.

* **Links:**
  Records can link to other records (creating graph structures). Intra-machine 
  (local) links are bidirectional to help avoid broken references, although 
  this protection is only within the same machine. These links enable the 
  modeling of non-tree relationships such as cross-references and bidirectional 
  pointers.

* **Identification:**
  Each local record is identified by its memory address (RAM address). But, 
  when transferred between machines (e.g., via serialization), records are 
  assigned a UUID to ensure global uniqueness. This mechanism supports 
  consistency in distributed environments.

* **Structure Flexibility:**
  CDP does not enforce a strict structure for child records. This grants great 
  flexibility, but **it is up to the programmer** to define and document the 
  conventions and protocols required by each agency (see Agencies).


#### Record Data

Records are binary data agnostic—any type of data may be stored, described only 
by a DT. Typical data types include binary numbers (various sizes), vectors, 
and UTF-8 text. This generic approach enables CDP to support a wide variety of 
applications without being tied to specific data formats.


### Poly-Structure: Dynamic Storage

Different applications have different storage needs. CDP provides multiple data 
structures to store records, allowing optimal choices based on usage patterns.

CDP uses multiple data structure types to store collections of records. The 
optimal storage for each record can be selected at runtime or load-time, and 
switched according to profiling.

* **Linked List:** Doubly-linked list for efficient insert/remove.

* **Dynamic Array:** Auto-growing, contiguous for fast lookups.

* **Packed Queue:** Fixed-length array chunks linked together; cache-friendly 
FIFO.

* **Red-Black Binary Tree:** Optimal for unpredictable dictionary sizes.

* **Simple Octree:** For spatial or catalog-type indexing.

The choice depends on the usage pattern and is transparent to the 
agent/programmer.

---


### Agents and Agencies

CDP processes data through modular, distributed components called Agents and 
their organizational groups called Agencies.


#### Agents

* **Definition:**
  Agents are executable code modules that process tasks from queues. Each agent 
  is registered with a specific **name (DT)** indicating the input it handles.

* **Inputs & Outputs:**
  Each agent receives tasks via an input queue and produces results via 
  outputs. An output is a generic term for sending results (the process of 
  converting results into new tasks).

* **Task Handling:**
  Agents process tasks as they arrive. If no agent matches a task name, the 
  system discards it. Agents are also free to ignore tasks as needed.

Agents encapsulate logic in a highly modular way, promoting code reuse, 
parallelism, and easier reasoning about system behavior.


#### Agencies

* **Definition:**
  An agency is a group of agents working together, typically sharing a DT and 
  handling related input queues. Each **agency instance** (or execution 
  instance) represents a single configuration of these agents, sharing certain 
  records and execution context.

* **Pipelines:**
  Agencies are linked into pipelines, where outputs of one agency become the 
  inputs of another, forming a cascading chain of processing steps.

* **Channels:**
  In CDP, a **channel** is any connection between agencies (i.e., between an 
  output and an input). Channels are managed by the CDP system—the user only 
  needs to use the API to connect the desired outputs and inputs.

This abstraction enables the creation of complex processing networks with 
minimal manual coordination.

---


### Tasks

The execution of work in CDP revolves around the concept of tasks. 
Understanding how tasks are formed, scheduled, and executed is critical to 
grasping the flow of data and logic through the system.

* **Definition:**
  A task is a message containing a target agency DT, an instance ID, an agent 
  DT (specifying the target input), and an optional record (message data) to 
  process.

* **Lifecycle:**
  Agents send results to outputs, which are then routed as new tasks to input 
  queues elsewhere in the pipeline.

* **Scheduling:**
  CDP operates in discrete passes. In each pass, all task queues are verified 
  for new tasks. Any new tasks generated are scheduled for the **next** 
  verification phase—never within the current pass. This enforces coherent, 
  deterministic execution and makes parallelism easy.

* **Storage:**
  Tasks and instances are themselves part of the local record hierarchy, with 
  unique addresses and IDs.

This design enforces predictability and simplifies debugging by ensuring that 
each execution step is cleanly separated from the next.

---


Execution Model
---------------

The following describes CDP's event-driven and pipeline-oriented execution 
strategy.

* The CDP system scans all agency input queues in each pass.

* When a task is found, the relevant agent (matched by DT) is invoked in a new 
thread if possible, receiving the agency instance, input (agent) name, and the 
task data.

* Agents may delegate to subroutines, and can be registered at startup or 
dynamically.

* If an agent produces a result, it creates new records for **output**, which 
are routed to downstream agencies as new tasks.

* Agents are oblivious to the global pipeline topology—they simply name which 
output to use for each result, and that's it.

This modular execution model facilitates deployment across heterogeneous 
systems.


### Inter-Pipeline Communication (IPLC)

Applications often need nested, hierarchical workflows. Sometimes, agents 
(clients) need to command pipelines they have spawned (owned pipelines), or 
receive status updates from these nested agents. CDP supports this by allowing 
pipelines to communicate and manage sub-pipelines in a structured way.

* **Command Tasks:**
  A client agent may send control commands such as "start", "pause", or "abort" 
  to its owned pipeline by sending a message (task) to a specific instance. 
  **However,** sub-agencies or further-nested pipelines will not automatically 
  propagate these commands unless this behaviour is explicitly implemented.

* **Response Tasks:**
  Nested agents may report "error", "log", or "fatal" events back to their 
  direct client via response tasks. If propagation up the hierarchy is needed, 
  explicit programming is required at each layer.

* **Protocols:**
  Both commands and responses should follow a pre-defined set, documented by 
  each agency or application.

This system of structured messaging provides the necessary hooks for building 
hierarchical control architectures.

---


CDP System Internals
--------------------

This section delves into implementation details, offering insight into CDP's 
internal architecture and how it achieves its performance and flexibility.

* **Implementation:**
  CDP is implemented in C for maximum portability (target: GCC).

* **Record Storage:**
  Records are minimal, featuring only a domain-tag name ID, metadata, and 
  pointers to actual data—efficient for rapid lookup and low memory use.


### Specialized Structures

* **Queues:**
  Records are sorted by insertion order using linked lists, arrays, or packed 
  queues. Multiple records with the same name but different content are 
  allowed.

* **Dictionaries:**
  Records are sorted by name using linked lists, arrays, or RB-trees. Duplicate 
  names overwrite previous entries.

* **Catalogs:**
  Collections of records sorted by a user function, with octrees available for 
  spatial indexing.

These internal strategies support the high-performance needs of real-time and 
resource-constrained environments.

---


Road Map
--------

| Step                             | Status |
| -------------------------------- | ------ |
| Basic record system              | ✅      |
| Unit testing for record system   | ✅      |
| Number and text naming           | ✅      |
| Dictionaries                     | ✅      |
| Catalogs                         | ✅      |
| Octree spatial indexing          | ✅      |
| Binary data types                | ✅      |
| Agency system (tasks & handling) | ⏳      |
| System agents                    | ⏳      |
| Record serialization             | ⏳      |
| File I/O XML storage             | ⏳      |
| LMDB storage integration         | ⏳      |
| Network implementation           | ⏳      |
| Network synchronization          | ⏳      |
| Access profiling & auditing      | ⏳      |
| Export to Emscripten             | ⏳      |
| Export to ESP32                  | ⏳      |
| Export to Arduino                | ⏳      |

---


Glossary
--------

For quick reference, here are brief definitions of the most important terms 
used throughout this document.

* **Agent:** Executable code that processes tasks from input queues.
* **Agency:** Group of agents working together, sharing instance/context.
* **Channel:** System-managed connection between two agencies.
* **Input:** A named task used for triggering a specific agent.
* **DT (Domain-Tag):** Pair that identifies the name/type/protocol of records.
* **Instance:** A specific configuration/context of an agency.
* **Output:** A DT name for results, to be linked to another agency.
* **Pipeline:** Linked sequence of agencies/agents processing data.
* **Record:** Fundamental data unit, can contain data and child records.
* **Task:** Message representing a unit of work for an agent.

