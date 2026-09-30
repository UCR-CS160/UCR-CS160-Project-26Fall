# UCR-CS160-Project-26Fall

## Project Overview

**Description**:  CS 160 is a project-driven course in which topics in concurrent programming and parallel systems are introduced through a quarter-long project. The project consists of multiple phases, which can also be viewed as a sequence of incremental projects—each building on the previous one and introducing more advanced features. The goal of this project is to motivate students to learn the importance of relevant topics and get hands-on experience with necessary design and development skills.

**Project Theme**:  The theme of this project is *a high-performance graph system* that supports multiple advanced features that involve parallel and concurrent operations. It also involves the discussion of *data locality and redundancy*, two critical aspects for optimizing the performance of parallel systems.

**Team Size**:  Each team consists of 3 students. If you have a strong desire to work in a 2-person team or single-person team, please make a request (the requirements would remain the same regardless of team size).

**Evaluation Criteria**: Each phase of this project is evaluated based on the follow criteria:

- *Correctness and Performance* (50% per team): The TA will evaluate the correctness and performance of the system using a set of test cases. Initially, a subset of these test cases will be released to students. Students then develop their own test cases to more thoroughly verify the correctness of their systems.   
    
  We expect the system to achieve reasonably good performance, relative to the hardware used. Students are encouraged to consult the TA in advance if they are unsure about the performance expectations.  
    
- *Report and Analysis* (50% per team): Students are required to document their development and experiments in a structured report ([template](REPORT_TEMPLATE.md)). The goal is to encourage a systematic approach to problem solving.

**Grades Distribution**:

| Phase (Tentative) | Start and Due Dates | Point Distribution |
| :---- | :---- | :---- |
| [Phase 1: concurrent local queries](phase-1/README.md) | 09/30 – 10/13 (2 weeks) | 8% |
| Phase 2: parallel global queries | 10/14 – 11/03 (3 weeks) | 11% \+ 3% |
| Phase 3: batched updates and incremental evaluation | 11/04 – 11/24 (3 weeks) | 11% \+ 3% |

**Late Submission Policy**:   
Each team can request a 3-day extension once without penalty. However, the following due dates *remain in place*. No further extensions will be granted after that.

## Project Rules

### Repository Structure and Submission

Organize each phase as follows:

```text
phase-1/
├── code/           # your implementation
└── report/
    ├── report.md   # or a PDF report
    └── images/     # figures used in the report, if any
phase-2/
├── code/
└── report/
phase-3/
├── code/
└── report/
```

I will grade the latest commit on the default branch as of the deadline. Make sure all code and report files are committed and pushed by then.

### Team Review

Each team will have a review for either Phase 2 or Phase 3; I will decide which teams are reviewed in which phase later. I will speak with the whole team and ask members questions in turn. Members may help each other, and the team will share one review result.

The review is primarily a chance to identify issues and help you improve. I will point out issues I find. For minor issues, I may ask you to revise the report to cover missing details or improve incomplete answers. I will deduct review-related points only for major issues (such as submitting AI-generated code or being unable to explain submitted code) or for ignoring requested revisions.

### Collaboration and AI Use

- Within your team, you may share code and solutions.
- Across teams, you may discuss ideas, but you may not share code.
- You may use AI to understand general issues, such as compilation errors or how to use an API. AI-generated code is not allowed. Disclose any AI use in your report, including the tool and what you used it for.
