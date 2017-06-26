![HyTech logo](https://hytechracing.gatech.edu/images/hytech_logo_small.png)

# 2017 Code

## Getting Started with this git repository
1. Download and install [Github Desktop](https://desktop.github.com/) (basic) or [git](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git) (advanced)
    * If you are using Github Desktop for the first time, be sure to read the [User Guide](https://help.github.com/desktop/guides/)
    * If you are using git for the first time, you should start off by reading a tutorial. There are many available online:
        * [Atlassian Git Tutorial](https://www.atlassian.com/git/tutorials/)
        * [Pro Git book](https://git-scm.com/book/en/v2)
    * You may want to keep a copy of [Github's Git Cheat Sheet](https://services.github.com/kit/downloads/github-git-cheat-sheet.pdf) as a reference
2. Create a Github account if you don't already have one
3. Talk to your subteam lead to be added to your respective team on the Github organization
4. Clone this repository to your computer
    * SSH: git@github.com:hytech-racing/code-2017.git
    * HTTPS: https://github.com/hytech-racing/code-2017.git
5. Make sure to refer to and contribute to the repository Wiki whenever you can
    * This is accessible in the repo toolbar (in between _Projects_ and _Pulse_)
    * _"In teaching others, we teach ourselves"_

## Organization
* `Archive/` Older code that have either been phased out due to design changes or code from older cars
* `Car_Code/` Code for the current car design we have
* `Libraries/` Library files that are needed to compile our code (you can create a symlink from your Arduino libraries folder to this)
* `Testing_Code/` Files that are used to either test components of the car or different hardware (such as the CAN Bus)
