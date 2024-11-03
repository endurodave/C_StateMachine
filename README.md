# State Machine Design in C
A compact C finite state machine (FSM) implementation that's easy to use on embedded and PC-based systems.

# Table of Contents

- [State Machine Design in C](#state-machine-design-in-c)
- [Table of Contents](#table-of-contents)
- [Preface](#preface)
  - [Related repositories](#related-repositories)
- [Introduction](#introduction)
  - [Background](#background)
- [Project Build](#project-build)
  - [Windows Visual Studio](#windows-visual-studio)
  - [Linux Make](#linux-make)
- [Why use a state machine?](#why-use-a-state-machine)
- [State machine design](#state-machine-design)
  - [Internal and external events](#internal-and-external-events)
  - [Event data](#event-data)
  - [State transitions](#state-transitions)
- [StateMachine module](#statemachine-module)
- [Motor example](#motor-example)
  - [External events](#external-events)
  - [State enumerations](#state-enumerations)
  - [State functions](#state-functions)
  - [State map](#state-map)
  - [State machine objects](#state-machine-objects)
  - [Transition map](#transition-map)
  - [New state machine steps](#new-state-machine-steps)
- [State engine](#state-engine)
- [Generating events](#generating-events)
- [No heap usage](#no-heap-usage)
- [CentrifugeTest example](#centrifugetest-example)
- [Multithread safety](#multithread-safety)
- [Conclusion](#conclusion)
- [References](#references)


# Preface

Originally published on CodeProject at: <a href="https://www.codeproject.com/Articles/1275479/State-Machine-Design-in-C"><strong>State Machine Design in C</strong></a>

Based on original design published in C\C++ Users Journal (Dr. Dobb's) at: <a href="http://www.drdobbs.com/cpp/state-machine-design-in-c/184401236"><strong>State Machine Design in C++</strong></a>

<p><a href="https://www.cmake.org/">CMake</a>&nbsp;is used to create the build files. CMake is free and open-source software. Windows, Linux and other toolchains are supported. See the <strong>CMakeLists.txt </strong>file for more information.</p>

## Related repositories

<ul>
    <li><a href="https://github.com/endurodave/C_StateMachineWithThreads">C Language State Machine with Threads</a> - by David Lafreniere</li>
	<li><a href="https://github.com/endurodave/C_Allocator">A Fixed Block Allocator in C</a> - by David Lafreniere</li>
    <li><a href="https://github.com/endurodave/StateMachine">State Machine Design in C++</a> - by David Lafreniere</li>
</ul>

# Introduction

<p>In 2000, I wrote an article entitled &quot;<em>State Machine Design in C++</em>&quot; for C/C++ Users Journal (R.I.P.). Interestingly, that old article is still available and (at the time of writing this article) the #1 hit on Google when searching for C++ state machine. The article was written over 15 years ago, but I continue to use the basic idea on numerous projects. It&#39;s compact, easy to understand and, in most cases, has just enough features to accomplish what I need.</p>

<p>Sometimes C is the right tool for the job. This article provides an alternate C language state machine implementation based on the ideas presented within the article &ldquo;<em>State Machine Design in C++</em>&rdquo;. The design is suitable for any platform, embedded or PC, with any C compiler. This state machine has the following features:</p>

<ul>
	<li><strong>C language</strong> &ndash; state machine written in C</li>
	<li><strong>Compact </strong>&ndash; consumes a minimum amount of resources.</li>
	<li><strong>Objects</strong>&nbsp;&ndash; support&nbsp;multiple instantiations of a&nbsp;single state machine type.</li>
	<li><strong>Transition tables</strong> &ndash; transition tables precisely control state transition behavior.</li>
	<li><strong>Events </strong>&ndash; every event is a simple function with any argument types.</li>
	<li><strong>State action</strong> &ndash; every state action is a separate function with a single, unique event data argument if desired.</li>
	<li><strong>Guards/entry/exit actions</strong>&nbsp;&ndash; optionally a state machine can use guard conditions and separate entry/exit action functions for each state.</li>
	<li><strong>Macros </strong>&ndash; optional multiline macro support simplifies usage by automating the code &quot;machinery&quot;.</li>
	<li><strong>Error checking</strong> &ndash; compile time and runtime checks catch mistakes early.</li>
	<li><strong>Thread-safe</strong> &ndash; adding software locks to make the code thread-safe is easy.</li>
</ul>

<p>The article is not a tutorial on the best design decomposition practices for software state machines. I&#39;ll be focusing on state machine code and simple examples with just enough complexity to facilitate understanding the features and usage.</p>

## Background

<p>A common design technique in the repertoire of most programmers is the venerable finite state machine (FSM). Designers use this programming construct to break complex problems into manageable states and state transitions. There are innumerable ways to implement a state machine.</p>

<p>A switch statement provides one of the easiest to implement and most common version of a state machine. Here, each case within the switch statement becomes a state, implemented something like:</p>

<pre lang="c++">
switch (currentState) {
   case ST_IDLE:
       // do something in the idle state
       break;

    case ST_STOP:
       // do something in the stop state
       break;

    // etc...
}</pre>

<p>This method is certainly appropriate for solving many different design problems. When employed on an event driven, multithreaded project, however, state machines of this form can be quite limiting.</p>

<p>The first problem revolves around controlling what state transitions are valid and which ones are invalid. There is no way to enforce the state transition rules. Any transition is allowed at any time, which is not particularly desirable. For most designs, only a few transition patterns are valid. Ideally, the software design should enforce these predefined state sequences and prevent the unwanted transitions. Another problem arises when trying to send data to a specific state. Since the entire state machine is located within a single function, sending additional data to any given state proves difficult. And lastly these designs are rarely suitable for use in a multithreaded system. The designer must ensure the state machine is called from a single thread of control.</p>

# Project Build

<a href="https://www.cmake.org">CMake</a> is used to create the build files. CMake is free and open-source software. Windows, Linux and other toolchains are supported. Example CMake console commands executed inside the project root directory: 

## Windows Visual Studio

<code>cmake -G "Visual Studio 17 2022" -A Win32 -B ../C_StateMachineBuild -S .</code>

After executed, open the Visual Studio project from within the <code>C_StateMachineBuild</code> directory.

## Linux Make

<code>cmake -G "Unix Makefiles" -B ../C_StateMachineBuild -S .</code>

After executed, build the software from within the C_StateMachineBuild directory using the command <code>make</code>. Run the console app using <code>./C_StateMachineApp</code>.

# Why use a state machine?

<p>Implementing code using a state machine is an extremely handy design technique for solving complex engineering problems. State machines break down the design into a series of steps, or what are called states in state-machine lingo. Each state performs some narrowly defined task. Events, on the other hand, are the stimuli, which cause the state machine to move, or transition, between states.</p>

<p>To take a simple example, which I will use throughout this article, let&#39;s say we are designing motor-control software. We want to start and stop the motor, as well as change the motor&#39;s speed. Simple enough. The motor control events to be exposed to the client software will be as follows:</p>

<ol>
	<li><strong>Set Speed</strong> &ndash; sets the motor going at a specific speed.</li>
	<li><strong>Halt</strong> &ndash; stops the motor.</li>
</ol>

<p>These events provide the ability to start the motor at whatever speed desired, which also implies changing the speed of an already moving motor. Or we can stop the motor altogether. To the motor-control module, these two events, or functions, are considered external events. To a client using our code, however, these are just plain functions.</p>

<p>These events are not state machine states. The steps required to handle these two events are different. In this case the states are:</p>

<ol>
	<li><strong>Idle </strong>&mdash; the motor is not spinning but is at rest.</li>
</ol>

<ul>
	<li>Do nothing.</li>
</ul>

<ol start="2">
	<li><strong>Start </strong>&mdash; starts the motor from a dead stop.</li>
</ol>

<ul>
	<li>Turn on motor power.</li>
	<li>Set motor speed.</li>
</ul>

<ol start="3">
	<li><strong>Change Speed </strong>&mdash; adjust the speed of an already moving motor.</li>
</ol>

<ul>
	<li>Change motor speed.</li>
</ul>

<ol start="4">
	<li><strong>Stop </strong>&mdash; stop a moving motor.</li>
</ol>

<ul>
	<li>Turn off motor power.</li>
	<li>Go to the Idle state.</li>
</ul>

<p>As can be seen, breaking the motor control into discreet states, as opposed to having one monolithic function, we can more easily manage the rules of how to operate the motor.</p>

<p>Every state machine has the concept of a &quot;current state.&quot; This is the state the state machine currently occupies. At any given moment in time, the state machine can be in only a single state. Every instance of a particular state machine instance can set the initial state when defined. That initial state, however, does not execute during object creation. Only an event sent to the state machine causes a state function to execute.</p>

<p>To graphically illustrate the states and events, we use a state diagram. Figure 1 below shows the state transitions for the motor control module. A box denotes a state and a connecting arrow indicates the event transitions. Arrows with the event name listed are external events, whereas unadorned lines are considered internal events. (I cover the differences between internal and external events later in the article.)</p>

<p><img alt="" src="Motor.png" /></p>

<div class="Caption">Figure 1: Motor state diagram</div>

<p>As you can see, when an event comes in the state transition that occurs depends on state machine&#39;s current state. When a SetSpeed event comes in, for instance, and the motor is in the Idle state, it transitions to the Start state. However, that same SetSpeed event generated while the current state is Start transitions the motor to the ChangeSpeed state. You can also see that not all state transitions are valid. For instance, the motor can&#39;t transition from ChangeSpeed to Idle without first going through the Stop state.</p>

<p>In short, using a state machine captures and enforces complex interactions, which might otherwise be difficult to convey and implement.</p>

# State machine design

## Internal and external events

<p>As I mentioned earlier, an event is the stimulus that causes a state machine to transition between states. For instance, a button press could be an event. Events can be broken out into two categories: external and internal. The external event, at its most basic level, is a function call into a state-machine module. These functions are public and are called from the outside or from code external to the state-machine object. Any thread or task within a system can generate an external event. If the external event function call causes a state transition to occur, the state will execute synchronously within the caller&#39;s thread of control. An internal event, on the other hand, is self-generated by the state machine itself during state execution.</p>

<p>A typical scenario consists of an external event being generated, which, again, boils down to a function call into the module&#39;s public interface. Based upon the event being generated and the state machine&#39;s current state, a lookup is performed to determine if a transition is required. If so, the state machine transitions to the new state and the code for that state executes. At the end of the state function, a check is performed to determine whether an internal event was generated. If so, another transition is performed and the new state gets a chance to execute. This process continues until the state machine is no longer generating internal events, at which time the original external event function call returns. The external event and all internal events, if any, execute within the caller&#39;s thread of control.</p>

<p>Once the external event starts the state machine executing, it cannot be interrupted by another external event until the external event and all internal events have completed execution if locks are used. This run to completion model provides a multithread-safe environment for the state transitions. Semaphores or mutexes can be used in the state machine engine to block other threads that might be trying to be simultaneously access the same state machine instance. See source code function <code>_SM_ExternalEvent()</code> comments for where the locks go.</p>

## Event data

<p>When an event is generated, it can optionally attach event data to be used by the state function during execution. Event data is a single <code>const</code> or non-<code>const </code>pointer to any built-in or user-defined data type.</p>

<p>Once the state has completed execution, the event data is considered used up and must be deleted. Therefore, any event data sent to a state machine must be dynamically created via <code>SM_XAlloc()</code>. &nbsp;The state machine engine automatically frees allocated event data using <code>SM_XFree()</code>.</p>

## State transitions

<p>When an external event is generated, a lookup is performed to determine the state transition course of action. There are three possible outcomes to an event: new state, event ignored, or cannot happen. A new state causes a transition to a new state where it is allowed to execute. Transitions to the existing state are also possible, which means the current state is re-executed. For an ignored event, no state executes. However, the event data, if any, is deleted. The last possibility, cannot happen, is reserved for situations where the event is not valid given the current state of the state machine. If this occurs, the software faults.</p>

<p>In this implementation, internal events are not required to perform a validating transition lookup. The state transition is assumed to be valid. You could check for both valid internal and external event transitions, but in practice, this just takes more storage space and generates busywork for very little benefit. The real need for validating transitions lies in the asynchronous, external events where a client can cause an event to occur at an inappropriate time. Once the state machine is executing, it cannot be interrupted. It is under the control of the private implementation, thereby making transition checks unnecessary. This gives the designer the freedom to change states, via internal events, without the burden of updating transition tables.</p>

# StateMachine module

<p>The state machine source code is contained within the <strong>StateMachine.c</strong> and <strong>StateMachine.h</strong> files. The code below shows the partial header. The <strong><code>StateMachine</code> </strong>header contains various preprocessor multiline macros to ease implementation of a state machine.</p>

<pre lang="c++">
enum { EVENT_IGNORED = 0xFE, CANNOT_HAPPEN = 0xFF };

typedef void NoEventData;

// State machine constant data
typedef struct
{
    const CHAR* name;
    const BYTE maxStates;
    const struct SM_StateStruct* stateMap;
    const struct SM_StateStructEx* stateMapEx;
} SM_StateMachineConst;

// State machine instance data
typedef struct 
{
    const CHAR* name;
    void* pInstance;
    BYTE newState;
    BYTE currentState;
    BOOL eventGenerated;
    void* pEventData;
} SM_StateMachine;

// Generic state function signatures
typedef void (*SM_StateFunc)(SM_StateMachine* self, void* pEventData);
typedef BOOL (*SM_GuardFunc)(SM_StateMachine* self, void* pEventData);
typedef void (*SM_EntryFunc)(SM_StateMachine* self, void* pEventData);
typedef void (*SM_ExitFunc)(SM_StateMachine* self);

typedef struct SM_StateStruct
{
    SM_StateFunc pStateFunc;
} SM_StateStruct;

typedef struct SM_StateStructEx
{
    SM_StateFunc pStateFunc;
    SM_GuardFunc pGuardFunc;
    SM_EntryFunc pEntryFunc;
    SM_ExitFunc pExitFunc;
} SM_StateStructEx;

// Public functions
#define SM_Event(_smName_, _eventFunc_, _eventData_) \
    _eventFunc_(&amp;_smName_##Obj, _eventData_)

// Protected functions
#define SM_InternalEvent(_newState_, _eventData_) \
    _SM_InternalEvent(self, _newState_, _eventData_)
#define SM_GetInstance(_instance_) \
    (_instance_*)(self-&gt;pInstance);

// Private functions
void _SM_ExternalEvent(SM_StateMachine* self, const SM_StateMachineConst* selfConst, BYTE newState, void* pEventData);
void _SM_InternalEvent(SM_StateMachine* self, BYTE newState, void* pEventData);
void _SM_StateEngine(SM_StateMachine* self, const SM_StateMachineConst* selfConst);
void _SM_StateEngineEx(SM_StateMachine* self, const SM_StateMachineConst* selfConst);

#define SM_DECLARE(_smName_) \
    extern SM_StateMachine _smName_##Obj; 

#define SM_DEFINE(_smName_, _instance_) \
    SM_StateMachine _smName_##Obj = { #_smName_, _instance_, \
        0, 0, 0, 0 }; 

#define EVENT_DECLARE(_eventFunc_, _eventData_) \
    void _eventFunc_(SM_StateMachine* self, _eventData_* pEventData);

#define EVENT_DEFINE(_eventFunc_, _eventData_) \
    void _eventFunc_(SM_StateMachine* self, _eventData_* pEventData)

#define STATE_DECLARE(_stateFunc_, _eventData_) \
    static void ST_##_stateFunc_(SM_StateMachine* self, _eventData_* pEventData);

#define STATE_DEFINE(_stateFunc_, _eventData_) \
    static void ST_##_stateFunc_(SM_StateMachine* self, _eventData_* pEventData)
</pre>

<p>The <code>SM_Event()</code> macro is used to generate external events whereas <code>SM_InternalEvent()</code> generates an internal event during state function execution. <code>SM_GetInstance()</code> obtains a pointer to the current state machine object.</p>

<p><code>SM_DECLARE </code>and <code>SM_DEFINE</code> are used to create a state machine instance. <code>EVENT_DECLARE</code> and <code>EVENT_DEFINE</code> create external event functions. And finally, <code>STATE_DECLARE</code> and <code>STATE_DEFINE</code> create state functions.</p>

# Motor example

<p><code>Motor </code>implements our hypothetical motor-control state machine, where clients can start the motor, at a specific speed, and stop the motor. The <code>Motor</code> header interface is shown below:</p>

<pre lang="c++">
#include &quot;StateMachine.h&quot;

// Motor object structure
typedef struct
{
    INT currentSpeed;
} Motor;

// Event data structure
typedef struct
{
    INT speed;
} MotorData;

// State machine event functions
EVENT_DECLARE(MTR_SetSpeed, MotorData)
EVENT_DECLARE(MTR_Halt, NoEventData)</pre>

<p>The <code>Motor</code> source file uses macros to simplify usage by hiding the required state machine machinery.</p>

<pre lang="c++">
// State enumeration order must match the order of state
// method entries in the state map
enum States
{
    ST_IDLE,
    ST_STOP,
    ST_START,
    ST_CHANGE_SPEED,
    ST_MAX_STATES
};

// State machine state functions
STATE_DECLARE(Idle, NoEventData)
STATE_DECLARE(Stop, NoEventData)
STATE_DECLARE(Start, MotorData)
STATE_DECLARE(ChangeSpeed, MotorData)

// State map to define state function order
BEGIN_STATE_MAP(Motor)
    STATE_MAP_ENTRY(ST_Idle)
    STATE_MAP_ENTRY(ST_Stop)
    STATE_MAP_ENTRY(ST_Start)
    STATE_MAP_ENTRY(ST_ChangeSpeed)
END_STATE_MAP(Motor)

// Set motor speed external event
EVENT_DEFINE(MTR_SetSpeed, MotorData)
{
    // Given the SetSpeed event, transition to a new state based upon 
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(ST_START)          // ST_Idle       
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_Stop       
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED)   // ST_Start      
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED)   // ST_ChangeSpeed
    END_TRANSITION_MAP(Motor, pEventData)
}

// Halt motor external event
EVENT_DEFINE(MTR_Halt, NoEventData)
{
    // Given the Halt event, transition to a new state based upon 
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_Idle
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_Stop
        TRANSITION_MAP_ENTRY(ST_STOP)           // ST_Start
        TRANSITION_MAP_ENTRY(ST_STOP)           // ST_ChangeSpeed
    END_TRANSITION_MAP(Motor, pEventData)
}</pre>

## External events

<p><code>MTR_SetSpeed </code>and <code>MTR_Halt</code> are considered external events into the <code>Motor</code> state machine. <code>MTR_SetSpeed </code>takes a pointer to <code>MotorData</code> event data, containing the motor speed. This data structure will be freed using <code>SM_XFree()</code> upon completion of the state processing, so it is imperative that it be created using <code>SM_XAlloc()</code> before the function call is made.</p>

## State enumerations

<p>Each state function must have an enumeration associated with it. These enumerations are used to store the current state of the state machine. In <code>Motor</code>, <code>States</code> provides these enumerations, which are used later for indexing into the transition map and state map lookup tables.</p>

## State functions

<p>State functions implement each state &mdash; one state function per state-machine state. <code>STATE_DECLARE </code>is used to declare the state function interface and <code>STATE_DEFINE </code>defines the implementation.</p>

<pre lang="c++">
// State machine sits here when motor is not running
STATE_DEFINE(Idle, NoEventData)
{
    printf(&quot;%s ST_Idle\n&quot;, self-&gt;name);
}

// Stop the motor 
STATE_DEFINE(Stop, NoEventData)
{
    // Get pointer to the instance data and update currentSpeed
    Motor* pInstance = SM_GetInstance(Motor);
    pInstance-&gt;currentSpeed = 0;

    // Perform the stop motor processing here
    printf(&quot;%s ST_Stop: %d\n&quot;, self-&gt;name, pInstance-&gt;currentSpeed);

    // Transition to ST_Idle via an internal event
    SM_InternalEvent(ST_IDLE, NULL);
}

// Start the motor going
STATE_DEFINE(Start, MotorData)
{
    ASSERT_TRUE(pEventData);

    // Get pointer to the instance data and update currentSpeed
    Motor* pInstance = SM_GetInstance(Motor);
    pInstance-&gt;currentSpeed = pEventData-&gt;speed;

    // Set initial motor speed processing here
    printf(&quot;%s ST_Start: %d\n&quot;, self-&gt;name, pInstance-&gt;currentSpeed);
}

// Changes the motor speed once the motor is moving
STATE_DEFINE(ChangeSpeed, MotorData)
{
    ASSERT_TRUE(pEventData);

    // Get pointer to the instance data and update currentSpeed
    Motor* pInstance = SM_GetInstance(Motor);
    pInstance-&gt;currentSpeed = pEventData-&gt;speed;

    // Perform the change motor speed here
    printf(&quot;%s ST_ChangeSpeed: %d\n&quot;, self-&gt;name, pInstance-&gt;currentSpeed);
}
</pre>

<p><code>STATE_DECLARE</code> and <code>STATE_DEFINE</code> use two arguments. The first argument is the state function name. The second argument is the event data type. If no event data is required, use <code>NoEventData</code>. Macros are also available for creating guard, exit and entry actions which are explained later in the article.</p>

<p>The <code>SM_GetInstance()</code> macro obtains an instance to the state machine object. The argument to the macro is the state machine name.</p>

<p>In this implementation, all state machine functions must adhere to these signatures, which are as follows:</p>

<pre lang="c++">
// Generic state function signatures
typedef void (*SM_StateFunc)(SM_StateMachine* self, void* pEventData);
typedef BOOL (*SM_GuardFunc)(SM_StateMachine* self, void* pEventData);
typedef void (*SM_EntryFunc)(SM_StateMachine* self, void* pEventData);
typedef void (*SM_ExitFunc)(SM_StateMachine* self);
</pre>

<p>Each <code>SM_StateFunc </code>accepts a pointer to a <code>SM_StateMachine</code> object and event data. If <code>NoEventData </code>is used, the <code>pEventData </code>argument will be <code>NULL</code>. Otherwise, the <code>pEventData</code> argument is of the type specified in <code>STATE_DEFINE</code>.</p>

<p>In <code>Motor</code>&rsquo;s <code>Start</code> state function, the <code>STATE_DEFINE(Start, MotorData)&nbsp;</code>macro expands to:</p>

<pre lang="c++">
void ST_Start(SM_StateMachine* self, MotorData* pEventData)</pre>

<p>Notice that every state function has <code>self </code>and&nbsp;<code>pEventData </code>arguments. <code>self </code>is a pointer to the state machine object and <code>pEventData </code>is the event data. Also note that the macro prepends &ldquo;ST_&rdquo; to the state name to create the function <code>ST_Start()</code>.</p>

<p>Similarly, the <code>Stop </code>state function <code>STATE_DEFINE(Stop, NoEventData)</code> is expands to:</p>

<pre lang="c++">
void ST_Stop(SM_StateMachine* self, void* pEventData)</pre>

<p><code>Stop </code>doesn&#39;t accept event data so the <code>pEventData </code>argument is <code>void*</code>.&nbsp;</p>

<p>Three characters are added to each state/guard/entry/exit function automatically within the macros. For instance, if declaring a function using <code>STATE_DEFINE(Idle, NoEventData)</code> the actual state function name is called <code>ST_Idle()</code>.</p>

<ol>
	<li>ST_ - state function prepend characters</li>
	<li>GD_ - guard function prepend characters</li>
	<li>EN_ - entry function prepend characters</li>
	<li>EX_ - exit function prepend characters</li>
</ol>

<p><code>SM_GuardFunc </code>and <code>SM_Entry </code>function <code>typedef</code>&rsquo;s also accept event data. <code>SM_ExitFunc </code>is unique in that no event data is allowed.</p>

## State map

<p>The state-machine engine knows which state function to call by using the state map. The state map maps the <code>currentState</code> variable to a specific state function. For instance, if <code>currentState </code>is 2, then the third state-map function pointer entry will be called (counting from zero). The state map table is created using these three macros:</p>

<div>
<pre>
BEGIN_STATE_MAP
STATE_MAP_ENTRY
END_STATE_MAP</pre>
</div>

<p><code>BEGIN_STATE_MAP </code>starts the state map sequence. Each <code>STATE_MAP_ENTRY </code>has a state function name argument. <code>END_STATE_MAP </code>terminates the map. The state map for <code>Motor </code>is shown below.</p>

<pre lang="c++">
BEGIN_STATE_MAP(Motor)
    STATE_MAP_ENTRY(ST_Idle)
    STATE_MAP_ENTRY(ST_Stop)
    STATE_MAP_ENTRY(ST_Start)
    STATE_MAP_ENTRY(ST_ChangeSpeed)
END_STATE_MAP
</pre>

<p>Alternatively, guard/entry/exit features require utilizing the <code>_EX</code> (extended) version of the macros.</p>

<pre lang="c++">
BEGIN_STATE_MAP_EX
STATE_MAP_ENTRY_EX or STATE_MAP_ENTRY_ALL_EX 
END_STATE_MAP_EX</pre>

<p>The <code>STATE_MAP_ENTRY_ALL_EX </code>macro has four arguments for the state action, guard condition, entry action and exit action in that order. The state action is mandatory but the other actions are optional. If a state doesn&#39;t have an action, then use 0 for the argument. If a state doesn&#39;t have any guard/entry/exit options, the <code>STATE_MAP_ENTRY_EX </code>macro defaults all unused options to 0. The macro snippet below is for an advanced example presented later in the article.</p>

<pre lang="c++">
// State map to define state function order
BEGIN_STATE_MAP_EX(CentrifugeTest)
    STATE_MAP_ENTRY_ALL_EX(ST_Idle, 0, EN_Idle, 0)
    STATE_MAP_ENTRY_EX(ST_Completed)
    STATE_MAP_ENTRY_EX(ST_Failed)
    STATE_MAP_ENTRY_ALL_EX(ST_StartTest, GD_StartTest, 0, 0)
    STATE_MAP_ENTRY_EX(ST_Acceleration)
    STATE_MAP_ENTRY_ALL_EX(ST_WaitForAcceleration, 0, 0, EX_WaitForAcceleration)
    STATE_MAP_ENTRY_EX(ST_Deceleration)
    STATE_MAP_ENTRY_ALL_EX(ST_WaitForDeceleration, 0, 0, EX_WaitForDeceleration)
END_STATE_MAP_EX(CentrifugeTest)</pre>

<p>Don&rsquo;t forget to add the prepended characters (ST_, GD_, EN_ or EX_) for each function.</p>

## State machine objects

<p>In C++, objects are integral to the language. Using C, you have to work a bit harder to accomplish similar behavior. This C language state machine supports multiple state machine objects (or instances) instead of having&nbsp;a single, static state machine implementation.</p>

<p>The <code>SM_StateMachine </code>data structure stores state machine instance data; one object per state machine instance. The&nbsp;<code>SM_StateMachineConst </code>data structure stores constant data; one constant object per state machine type.</p>

<p>The state machine is defined using <code>SM_DEFINE </code>macro. The first argument is the state machine name. The second argument is a pointer to a user defined state machine structure, or <code>NULL </code>if no user object.</p>

<pre lang="c++">
#define SM_DEFINE(_smName_, _instance_) \
    SM_StateMachine _smName_##Obj = { #_smName_, _instance_, \
        0, 0, 0, 0 };</pre>

<p>In this example, the state machine name is <code>Motor</code> and two objects and two state machines are created.</p>

<pre lang="c++">
// Define motor objects
static Motor motorObj1;
static Motor motorObj2;

// Define two public Motor state machine instances
SM_DEFINE(Motor1SM, &amp;motorObj1)
SM_DEFINE(Motor2SM, &amp;motorObj2)</pre>

<p>Each motor object handles state execution independent of the other. The <code>Motor </code>structure is used to store state machine instance-specific data. Within a state function, use <code>SM_GetInstance()</code>&nbsp;to obtain&nbsp;a pointer to the <code>Motor </code>object at runtime.</p>

<pre lang="c++">
// Get pointer to the instance data and update currentSpeed
Motor* pInstance = SM_GetInstance(Motor);
pInstance-&gt;currentSpeed = pEventData-&gt;speed;
</pre>

## Transition map

<p>The last detail to attend to are the state transition rules. How does the state machine know what transitions should occur? The answer is the transition map. A transition map is lookup table that maps the <code>currentState </code>variable to a state enum constant. Every external event function has a transition map table created with three macros:</p>

<pre lang="c++">
BEGIN_TRANSITION_MAP
TRANSITION_MAP_ENTRY
END_TRANSITION_MAP
</pre>

<p>The <code>MTR_Halt </code>event function in <code>Motor </code>defines the transition map as:</p>

<pre lang="c++">
// Halt motor external event
EVENT_DEFINE(MTR_Halt, NoEventData)
{
    // Given the Halt event, transition to a new state based upon 
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_Idle
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_Stop
        TRANSITION_MAP_ENTRY(ST_STOP)           // ST_Start
        TRANSITION_MAP_ENTRY(ST_STOP)           // ST_ChangeSpeed
    END_TRANSITION_MAP(Motor, pEventData)
}
</pre>

<p><code>BEGIN_TRANSITION_MAP</code> starts the map. Each <code>TRANSITION_MAP_ENTRY</code> that follows indicates what the state machine should do based upon the current state. The number of entries in each transition map table must match the number of state functions exactly. In our example, we have four state functions, so we need four transition map entries. The location of each entry matches the order of state functions defined within the state map. Thus, the first entry within the <code>MTR_Halt</code> function indicates an <code>EVENT_IGNORED </code>as shown below.</p>

<pre lang="c++">
TRANSITION_MAP_ENTRY (EVENT_IGNORED)&nbsp;&nbsp;&nbsp; <i>// ST_Idle</i></pre>

<p>This is interpreted as &quot;If a Halt event occurs while the current state is state Idle, just ignore the event.&quot;</p>

<p>Similarly, the third entry in the map is:</p>

<div>
<pre>
TRANSITION_MAP_ENTRY (ST_STOP)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <em>// ST_Start</em></pre>
</div>

<p>This indicates &quot;If a Halt event occurs while current is state Start, then transition to state Stop.&quot;</p>

<p><code>END_TRANSITION_MAP </code>terminates the map. The first argument to this macro is the state machine name. The second argument is the event data.</p>

<p>The <code>C_ASSERT()</code> macro is used within <code>END_TRANSITION_MAP</code>. If there is a mismatch between the number of state machine states and the number of transition map entries, a compile time error is generated.</p>

## New state machine steps

<p>Creating a new state machine requires a few basic high-level steps:</p>

<ol>
	<li>Create a <code>States </code>enumeration with one entry per state function.</li>
	<li>Define state functions.</li>
	<li>Define event functions.</li>
	<li>Create one state map lookup table using the <code>STATE_MAP</code> macros.</li>
	<li>Create one transition map lookup table for each external event function using the <code>TRANSITION_MAP </code>macros.</li>
</ol>

# State engine

<p>The state engine executes the state functions based upon events generated. The transition map is an array of <code>SM_StateStruct</code> instances indexed by the <code>currentState </code>variable. When the <code>_SM_StateEngine()</code> function executes, it looks up the correct state function within the <code>SM_StateStruct </code>array. After the state function has a chance to execute, it frees the event data, if any, before checking to see if any internal events were generated via <code>SM_InternalEvent()</code>.</p>

<pre lang="c++">
// The state engine executes the state machine states
void _SM_StateEngine(SM_StateMachine* self, SM_StateMachineConst* selfConst)
{
&nbsp; &nbsp; void* pDataTemp = NULL;

&nbsp; &nbsp; ASSERT_TRUE(self);
&nbsp; &nbsp; ASSERT_TRUE(selfConst);

&nbsp; &nbsp; // While events are being generated keep executing states
&nbsp; &nbsp; while (self-&gt;eventGenerated)
&nbsp; &nbsp; {
&nbsp; &nbsp; &nbsp; &nbsp; // Error check that the new state is valid before proceeding
&nbsp; &nbsp; &nbsp; &nbsp; ASSERT_TRUE(self-&gt;newState &lt; selfConst-&gt;maxStates);

&nbsp; &nbsp; &nbsp; &nbsp; // Get the pointers from the state map
&nbsp; &nbsp; &nbsp; &nbsp; SM_StateFunc state = selfConst-&gt;stateMap[self-&gt;newState].pStateFunc;

&nbsp; &nbsp; &nbsp; &nbsp; // Copy of event data pointer
&nbsp; &nbsp; &nbsp; &nbsp; pDataTemp = self-&gt;pEventData;

&nbsp; &nbsp; &nbsp; &nbsp; // Event data used up, reset the pointer
&nbsp; &nbsp; &nbsp; &nbsp; self-&gt;pEventData = NULL;

&nbsp; &nbsp; &nbsp; &nbsp; // Event used up, reset the flag
&nbsp; &nbsp; &nbsp; &nbsp; self-&gt;eventGenerated = FALSE;

&nbsp; &nbsp; &nbsp; &nbsp; // Switch to the new current state
&nbsp; &nbsp; &nbsp; &nbsp; self-&gt;currentState = self-&gt;newState;

&nbsp; &nbsp; &nbsp; &nbsp; // Execute the state action passing in event data
&nbsp; &nbsp; &nbsp; &nbsp; ASSERT_TRUE(state != NULL);
&nbsp; &nbsp; &nbsp; &nbsp; state(self, pDataTemp);

&nbsp; &nbsp; &nbsp; &nbsp; // If event data was used, then delete it
&nbsp; &nbsp; &nbsp; &nbsp; if (pDataTemp)
&nbsp; &nbsp; &nbsp; &nbsp; {
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; SM_XFree(pDataTemp);
&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; pDataTemp = NULL;
&nbsp; &nbsp; &nbsp; &nbsp; }
&nbsp; &nbsp; }
}
</pre>

<p>The state engine logic for guard, entry, state, and exit actions is expressed by the following sequence. The <code>_SM_StateEngine()</code> engine implements only #1 and #5 below. The extended <code>_SM_StateEngineEx()</code> engine uses the entire logic sequence.</p>

<ol>
	<li>Evaluate the state transition table. If <code>EVENT_IGNORED</code>, the event is ignored and the transition is not performed. If <code>CANNOT_HAPPEN</code>, the software faults. Otherwise, continue with next step.</li>
	<li>If a guard condition is defined execute the guard condition function. If the guard condition returns <code>FALSE</code>, the state transition is ignored and the state function is not called. If the guard returns <code>TRUE</code>, or if no guard condition exists, the state function will be executed.</li>
	<li>If transitioning to a new state and an exit action is defined for the current state, call the current state exit action function.</li>
	<li>If transitioning to a new state and an entry action is defined for the new state, call the new state entry action function.</li>
	<li>Call the state action function for the new state. The new state is now the current state.</li>
</ol>

# Generating events

<p>At this point, we have a working state machine. Let&#39;s see how to generate events to it. An external event is generated by dynamically creating the event data structure using <code>SM_XAlloc()</code>, assigning the structure member variables, and calling the external event function using the <code>SM_Event()</code> macro. The following code fragment shows how a synchronous call is made.</p>

<pre lang="c++">
MotorData* data;
 
// Create event data
data = SM_XAlloc(sizeof(MotorData));
data-&gt;speed = 100;

// Call MTR_SetSpeed event function to start motor
SM_Event(Motor1SM, MTR_SetSpeed, data);
</pre>

<p>The <code>SM_Event()</code> first argument is the state machine name. The second argument is the event function to invoke. The third argument is the event data, or <code>NULL </code>if no data.</p>

<p>To generate an internal event from within a state function, call <code>SM_InternalEvent()</code>. If the destination doesn&#39;t accept event data, then the last argument is <code>NULL</code>. Otherwise, create the event data using <code>SM_XAlloc()</code>.</p>

<pre lang="c++">
SM_InternalEvent(ST_IDLE, NULL);</pre>

<p>In the example above, once the state function completes execution the state machine will transition to the <code>ST_Idle</code> state. If, on the other hand, event data needs to be sent to the destination state, then the data structure needs to be created on the heap and passed in as an argument.</p>

<pre lang="c++">
MotorData* data;    
data = SM_XAlloc(sizeof(MotorData));
data-&gt;speed = 100;
SM_InternalEvent(ST_CHANGE_SPEED, data);
</pre>

# No heap usage

<p>All state machine event data must be dynamically created. However, on some systems using the heap is undesirable. The included <code>x_allocator</code> module is a fixed block memory allocator that eliminates heap usage. Define <code>USE_SM_ALLOCATOR </code>within <strong>StateMachine.c</strong> to use the fixed block allocator. See the <strong>References</strong> section below for&nbsp;<code>x_allocator</code> information.</p>

# CentrifugeTest example

<p>The <code>CentrifugeTest </code>example shows how an extended state machine is created using guard, entry and exit actions. The state diagram is shown below.</p>

<p><img alt="" src="CentrifugeTest.png" style="height: 736px; width: 500px" /></p>

<div class="Caption">Figure 2: CentrifugeTest state diagram</div>

<p>A <code>CentrifgeTest </code>object and state machine is created. The only difference here is that the state machine is a singleton, meaning the object is private and only one instance of <code>CentrifugeTest </code>can be created. This is unlike the <code>Motor </code>state machine where multiple instances are allowed.</p>

<pre lang="c++">
// CentrifugeTest object structure
typedef struct
{
    INT speed;
    BOOL pollActive;
} CentrifugeTest;

// Define private instance of motor state machine
CentrifugeTest centrifugeTestObj;
SM_DEFINE(CentrifugeTestSM, &amp;centrifugeTestObj)
</pre>

<p>The extended state machine uses <code>ENTRY_DECLARE</code>, <code>GUARD_DECLARE</code> and <code>EXIT_DECLARE </code>macros.</p>

<pre lang="c++">
// State enumeration order must match the order of state
// method entries in the state map
enum States
{
    ST_IDLE,
    ST_COMPLETED,
    ST_FAILED,
    ST_START_TEST,
    ST_ACCELERATION,
    ST_WAIT_FOR_ACCELERATION,
    ST_DECELERATION,
    ST_WAIT_FOR_DECELERATION,
    ST_MAX_STATES
};

// State machine state functions
STATE_DECLARE(Idle, NoEventData)
ENTRY_DECLARE(Idle, NoEventData)
STATE_DECLARE(Completed, NoEventData)
STATE_DECLARE(Failed, NoEventData)
STATE_DECLARE(StartTest, NoEventData)
GUARD_DECLARE(StartTest, NoEventData)
STATE_DECLARE(Acceleration, NoEventData)
STATE_DECLARE(WaitForAcceleration, NoEventData)
EXIT_DECLARE(WaitForAcceleration)
STATE_DECLARE(Deceleration, NoEventData)
STATE_DECLARE(WaitForDeceleration, NoEventData)
EXIT_DECLARE(WaitForDeceleration)

// State map to define state function order
BEGIN_STATE_MAP_EX(CentrifugeTest)
    STATE_MAP_ENTRY_ALL_EX(ST_Idle, 0, EN_Idle, 0)
    STATE_MAP_ENTRY_EX(ST_Completed)
    STATE_MAP_ENTRY_EX(ST_Failed)
    STATE_MAP_ENTRY_ALL_EX(ST_StartTest, GD_StartTest, 0, 0)
    STATE_MAP_ENTRY_EX(ST_Acceleration)
    STATE_MAP_ENTRY_ALL_EX(ST_WaitForAcceleration, 0, 0, EX_WaitForAcceleration)
    STATE_MAP_ENTRY_EX(ST_Deceleration)
    STATE_MAP_ENTRY_ALL_EX(ST_WaitForDeceleration, 0, 0, EX_WaitForDeceleration)
END_STATE_MAP_EX(CentrifugeTest)
</pre>

<p>Notice the <code>_EX</code> extended state map macros so the guard/entry/exit features are supported. Each guard/entry/exit <code>DECLARE </code>macro must be matched with the <code>DEFINE</code>. For instance, a guard condition for the <code>StartTest </code>state function is declared as:</p>

<pre lang="c++">
GUARD_DECLARE(StartTest, NoEventData)</pre>

<p>The guard condition function returns <code>TRUE </code>if the state function is to be executed or <code>FALSE </code>otherwise.</p>

<pre lang="c++">
// Guard condition to determine whether StartTest state is executed.
GUARD_DEFINE(StartTest, NoEventData)
{
    printf(&quot;%s GD_StartTest\n&quot;, self-&gt;name);
    if (centrifugeTestObj.speed == 0)
        return TRUE;    // Centrifuge stopped. OK to start test.
    else
        return FALSE;   // Centrifuge spinning. Can&#39;t start test.
}
</pre>

# Multithread safety    

<p>To prevent preemption by another thread when the state machine is in the process of execution, the <code>StateMachine </code>module can use locks within the <code>_SM_ExternalEvent()</code>&nbsp;function. Before the external event is allowed to execute, a semaphore can be locked. When the external event and all internal events have been processed, the software lock is released, allowing another external event to enter the state machine instance.</p>

<p>Comments indicate where the lock and unlock should be placed if the application is multithreaded&nbsp;<em>and</em> mutiple threads are able to access a single state machine instance. Note that each <code>StateMachine </code>object should have its own instance of a software lock. This prevents a single instance from locking and preventing all other <code>StateMachine </code>objects from executing. Software locks are only required if a <code>StateMachine </code>instance is called by multiple threads of control. If not, then locks are not required.</p>

<ul>
</ul>

# Conclusion

<p>Implementing a state machine using this method as opposed to the old switch statement style may seem like extra effort. However, the payoff is in a more robust design that is capable of being employed uniformly over an entire multithreaded system. Having each state in its own function provides easier reading than a single huge <code>switch</code> statement, and allows unique event data to be sent to each state. In addition, validating state transitions prevents client misuse by eliminating the side effects caused by unwanted state transitions.</p>

<p>This C language version is a close translation of the&nbsp;C++ implementation I&rsquo;ve used for many years on different projects. Consider the C++ implementation within the <strong>References </strong>section if using C++.</p>

# References

<ul>
    <li><a href="https://github.com/endurodave/C_StateMachineWithThreads">C Language State Machine with Threads</a> - by David Lafreniere</li>
	<li><a href="https://github.com/endurodave/C_Allocator">A Fixed Block Allocator in C</a> - by David Lafreniere</li>
    <li><a href="https://github.com/endurodave/StateMachine">State Machine Design in C++</a> - by David Lafreniere</li>
</ul>
