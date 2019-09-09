#include<bits/stdc++.h>
using namespace std;
int cd(char* tokenlist[])
{

    //cout<<tokenlist[1]<<endl;
	if (tokenlist[1] == NULL)
    {
		chdir(getenv("HOME"));
		return 1;
	}

	else if (strcmp(tokenlist[1],"~")==0)
    {
        //cout<<"cd"<<endl;
		chdir(getenv("HOME"));
		return 1;
	}

	else{
		if (chdir(tokenlist[1]) == -1)
		{
            cout<<tokenlist[1]<<" "<<": No such directory exist "<<endl;
            return -1;
		}
	}
	return 0;
}
