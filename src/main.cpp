#include <iostream>

using namespace std;

void runBouncyBubble(bool shader);
void runBouncyBall(bool gravity);

int main() {
    // cout << "Running Bouncy Balls" << endl;
    // runBouncyBall(false);
    // cout << "Running Bouncy Balls with Gravity" << endl;
    // runBouncyBall(true);
    cout << "Running Bouncy Bubbles" << endl;
    runBouncyBubble(false);
    // cout << "Running Bouncy Bubbles with Shaders" << endl;
    // runBouncyBubble(true);
    return 0;
}