# Deployment
This document outlines how new code is deployed to the central robotmoose.com server. For self-hosted instances, this does not apply.

## Overview
1. Code is pushed to GitHub.
2. GitHub triggers a webhook, which notifies [Nova Alces](https://github.com/finetralfazz/nova_alces) of the change. This notification also specifies which branch was modified. The remaining steps are taken by Nova Alces on the relevant environment (test|production).
3. Changes are pulled from GitHub.
4. Superstar is recompiled and restarted, if necessary.

## Branches
This project has two primary branches. `master` is the development branch, and changes to it are deployed to [test.robotmoose.com](https://test.robotmoose.com). `production` is (as its name implies) the production branch, and any code merged into it will be deployed to [robotmoose.com](https://robotmoose.com). Be _VERY_ careful when deploying to production. 

## Other Notes
It might be worth enforcing code review before deployment to production (for example, requiring a pull request to be accepted by a different member of the project). As of right now, any project member can freely push to production.

The test and production instances currently run on the same server. This will likely be changed after benchmarking work is complete and we have a spare.
