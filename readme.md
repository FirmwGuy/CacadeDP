Cascade Data Processing
=======================

CascadeDP is MIT licensed distributed execution solution targeted to all
kind of platforms, including microcontrollers and embedded systems.

**This project is under active development in experimental state and not ready
for utilization yet!**

Inspired by Plan 9, everything is mapped into a virtual record system. The
underlying grouping is made of flexible data structures (poly-structure) that
may be switched on the fly according to run-time profiling. For example, a
dictionary may be set to use a linked list, an array or a red-black binary tree
as a record container.

Cascade processing is done using agents that act in behalf of a specific
organization of data and records. Agents may process local or remote records
and advertise their availability in the distributed network.

Remote public records are mapped (mounted) as local ones as they become
available. In this way no external central authority is needed.


Records
-------

Records are the fundamental data unit and may contain other records as
children. Records always have a name which make them suitable for indexing in a
dictionary kind of storing if needed. Names may be text or numbers. Names may
also be set arbitrarily or may be automatically assigned by the system. Since
records have names and may contain other records, a tree like hierarchy is
naturally achieved.

Records may also point to other records (links) so a graph kind of topology is
also possible. Local links always imply backward linking, so a record pointing
to another record means that the other record has link table with an entry
pointing back. In this manner broken links are prevented.

Records are binary data agnostic, so they may hold data of all kinds requiring
only a "domain" and a "tag" to identify the content. Data structuring is done
by using naming protocols and parent-child relationships, but there is no
enforcing in having a rigid structure since parent records may have all kinds
of children records.

This lack of structure enforcement for children allows great flexibility, since
one record may be used in several contexts where different processors look for
those children satisfying their own needs.

Besides having a full (name) path, all local (intra-machine) records have a
unique binary identifier that is automatically converted into a UUID in case of
inter-machine record transfers. This way, records always have a known address
and an unique ID in the network.


Agents
------

Agents are executable code selected by a domain-tag pair (DT) of a specific
queue, and become active (run) when the queue has a task to be processed. Tasks
are messages that contain an agency instance ID, a DT telling the input queue
and a record with information about the task to be performed.

A named instance input is the DT specified queue where tasks are stored. Each
input queue may have several "channels" connected to it but no the other way
around. Each channel is just an associated named output in some other agency
instance.

The task may generate result records which become available in a DT specified
output. From the point of view of the agent handling the task, an output is
just a name used to send results of the processing. I reality, the output is a
input queue where tasks are sent, so each output result becomes the task of
another agent, and so the data cascading takes effect.

When several agents are grouped together to implement a group logic over
different input queues they become an agency. Agencies are just a set of agents
(per-input executable code) grouped under the same DT. Execution instances are
records shared by all agents associated with such instance, so they are defined
in a per-agency basis. Records set by an agent are available to all other
agents using that same instance.

The typical processing pipeline configuration implies the creation (or loading)
of agency instances for the agencies to be used (a pipeline may have several
instances of the same agency DT). Then, all agency instances are connected as
needed so each named instance output become linked to another named instance
input.

Finally a signal is sent to the pipeline so the data generators may start
feeding the other agents with tasks to process. Since agencies and agents may
have their own pipelines the data flow becomes nested.


Tasks
-----

Tasks are created when an agent sends a result to one of its DT named outputs.
Then the system routes that result record to the connected input queue in that
pipeline.

Since tasks are verified by the system once for all agencies there is a
coherent execution of all tasks, where new tasks are never executed at the same
pass that previous tasks are, instead the new tasks are scheduled for the next
verification phase.

Also, since tasks are bound to an agency instance, any agent (located anywhere
in the network) may pick up one task and process it under such instance.
Parallelism happens effortless.

Tasks are always stored as part of the local record hierarchy, so they always
have a full address and ID.


Execution
---------

The system checks all agency queues for new tasks. When one is found it look up
the DT name of the targeted input; we call this input the "consumption". Next,
the agent registered to that DT name is called and executed (in a new thread if
possible) passing the agency instance, the consumption name and the task as
arguments.

The same agent (code) may be executed to carry several tasks, while a single
task may delegate the work to sub-routines according to the content of the
received record. There is no other obligation in the implementation of agents
other than register them at system start up.

If no agent is found for that consumption, the task is just ignored. Also, each
agent is free to just ignore any processing task at will.


Data Processing
---------------

Once a task is being handled by an agent it may produce a result, that is, the
creation of new records and their distribution to any connected agency down the
pipeline. We call this output channels the "production". They must be unique
according to a defined protocol so any other agent in the agency may use them
as well.

When a result record is created it is stored in the agency instance until it is
ready to be sent to a specific production. Once this is done, the system search
for any agency connected to such production (if not found, the record is just
discarded). Then, creates a new task with required information including the
target agency instance and the new outbound record. The task will be processed
at later system passes.

The agent (executable) processing the task has no need to know anything about
the topology of the pipeline. It only needs to know which production to use for
resulting records.


Inter-pipeline Communication
----------------------------

Since nested pipelines are possible, agents that created nested instance agents
need a way to command them, while them in turn need a way to report status
information back to their masters. We call the creator agents the "client".

Clients may send commands such as "start", "pause" or "abort", while nested
agents may send responses such as "error", "log" or "fatal". Such messages are
really tasks added to the respective agent, so they follow the same route as
conventional tasks.

When a client send a command it may be broadcast to all or just some of the
nested agents. Sub-agents (agents created by nested agents) will not be
notified, so proper command cascading down the processing hierarchy need to be
explicitly implemented.

In a similar fashion, reports coming from nested agents toward the client will
only reach that client. If that report is required to go upward to the
super-client it needs an explicit client implementation.

Both commands and responses are quite arbitrary so they need to be part of a
pre-defined set specified in the agency documentation.


System Internals
----------------

Cascade Data Processing is implemented in C to maximize its portability across
different platforms. It is intended to be used with GCC for such effect.

Record themselves are just entries in some parent store, using a
poly-structure. The record is minimal to enhance look up time, having only a
name ID, some metadata and a couple of pointers to the actual data.

### Poly-structure

The record system is implemented using any of the following storage techniques.

* A linked list: a double linked list queue targeted to efficient push, pop
and arbitrary remove operations.

* An array: a dynamic (automatically growing) array of continuous records,
ideal for static collections and fast look ups.

* A packed queue: a fixed-length array that may be linked to other chunks
forming a sequence, making it more cache friendly for sequential FIFO
processing than linked lists.

* A red-black binary tree: the best choice for dictionaries of unknown length.

* A simple octree: a very simple spatial indexing for catalog type of look-ups.

The choice of record storage type depends of the intended use of such records.
It may also be selected at run/load time using record access profiling.

### Parent Stores

Any record may have children records. This is achieved by adding a parent store
information to such record, including a poly-structure type, current number of
children, etc.

The child record itself may have a DT indicating the protocol it belongs to, so
when accessing the parent record proper casting may filter out other brother
records with the same name (if any) but different meaning.

### Binary Data

Any record may also have data, that is, opaque binary data whose meaning and
handling is responsibility of the respective accessing agent. A DT indicates
the type of data, while other binary metadata indicate its size and whether
it may be modified or not.

Typical data types provided by the system include native binary numbers (integers
and floats of several bit sizes), vectors of numbers (contiguous memory
regions) and text coded in UTF8.

### Queues

Queues are implemented by sorting records by their insertion order. Only linked
lists, arrays and packed queues are used for this. Records with the same name
but different content are allowed.

### Dictionaries

Dictionaries are automatically implemented by sorting the records by their
name. Only linked lists, arrays and RB-trees are used for this. If a record
with the same name is found at insertion time, it is overwritten.

### Catalogs

Catalogs are collections of records sorted by a user-provided function. The
poly-structure requirements are similar to dictionaries, with the added benefit
of having octrees.

Users needing sorting-by-key are required to store such keys somewhere inside
the respective record.


Road Map
--------

1. Basic record system.  
   Done.

2. Unit testing for record system.  
   Done.

3. Number and text record naming convention.  
   Done.

4. Dictionaries.   
   Done.

5. Catalogs.   
   Done.

6. Octree spatial indexing.
   Done.
   
7. Binary data types.
   Done.

8. Agency system with task execution and handling.

9. System agents, such as: 'system-step', 'buffer', 'cloner' or 'synchronizer'.

10. Record serialization.

11. File I/O storage for records using XML.

12. LMDB storage for records.

13. Network implementation, with advertising and connection.

14. Push/pull network data synchronization.

15. Record access profiling: create (optional) access time entries on each
record to keep track of access time. Audit such records in a regular basis and
write that information in a access log file.






