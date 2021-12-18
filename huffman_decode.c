#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define GET_BYTE(vbyte, index) (((vbyte) & (1 << ((index) ^ 7))) != 0)
typedef struct ascii_freq {
    unsigned char ascii;
    int freq;
}ascii_freq;
typedef struct huffman_tree{
    struct huffman_tree *parent,*lchild,*rchild;
    unsigned char ascii;
    int weight;
}huffT;
typedef struct huffman_code{
    unsigned char ascii;
    unsigned char *code;
}huffC;
//读取头文件数据
ascii_freq *head_data(char*filename,unsigned char*suffix,int*n_pointer,int*dis_p){
    FILE*f=fopen(filename,"rb");
    fseek(f,0,0);
    fread(suffix,sizeof(unsigned char),1,f);
    int i;
    for(i=1;suffix[i-1]!='\0';i++)fread(&suffix[i],1,1,f);
    fread(n_pointer,sizeof(int),1,f);
    ascii_freq *frequency=malloc(sizeof(ascii_freq)*(*n_pointer));
    fread(frequency,sizeof(ascii_freq),*n_pointer,f);
    fclose(f);
    *dis_p=sizeof(char)*i+sizeof(int)+sizeof(ascii_freq)*(*n_pointer);
    return frequency;
}
//构建Huffman树
void search_node(huffT*HT,int n,int *s1,int*s2){//最小权重为*s1,次小权重为*s2
    *s1=-1;
    *s2=-1;
    for(int i=0;i<n;i++){
        if(HT[i].parent!=NULL)continue;
        else if(*s1==-1)*s1=i;
        else if(*s2==-1)if(HT[i].weight>=HT[*s1].weight)*s2=i;
            else {*s2=*s1;*s1=i;}
        else if(HT[*s2].weight>HT[i].weight)if(HT[*s1].weight>HT[i].weight){*s2=*s1;*s1=i;}
            else *s2=i;
    }
}
huffT *cre_huffT(ascii_freq*frequency,int n){
    int m=2*n-1;
    huffT*HT=malloc(sizeof(huffT)*m);
    int i=0;
    for(;i<n;i++){//initialize:1~n
        HT[i].weight=frequency->freq;
        HT[i].parent=NULL;
        HT[i].lchild=NULL;
        HT[i].rchild=NULL;
        HT[i].ascii=frequency->ascii;
        frequency++;
    }
    for(;i<m;i++){//initialize:n+1~2n-1
        HT[i].weight=0;
        HT[i].parent=NULL;
        HT[i].lchild=NULL;
        HT[i].rchild=NULL;
        HT[i].ascii='\0';
        frequency++;
    }
    int *s1_p,*s2_p;
    int s1,s2;
    s1_p=&s1;
    s2_p=&s2;
    for(i=n;i<m;i++){
        search_node(HT,i,s1_p,s2_p);//find the two least node in 0~i-1
        HT[s1].parent=&HT[i];
        HT[s2].parent=&HT[i];
        HT[i].lchild=&HT[s1];
        HT[i].rchild=&HT[s2];
        HT[i].weight=HT[s1].weight+HT[s2].weight;
    }
    return HT;
}
//decode
void decode(char*filename,char*originname,huffT*root,int distance){
    FILE*fpIn=fopen(filename,"rb");
    FILE*fpOut=fopen(originname,"wb");
    fseek(fpIn,distance,0);
    unsigned char value;
    int index=0;
    huffT *now_node=root;
    fread(&value,sizeof(unsigned char),1,fpIn);
    while(!feof(fpIn)){
        for(;index<8&&now_node->lchild!=NULL;index++)now_node=GET_BYTE(value,index)?now_node->rchild:now_node->lchild;
        if(index==8){index=0;
        fread(&value,sizeof(unsigned char),1,fpIn);}
        if(now_node->lchild==NULL){
            fwrite(&now_node->ascii,sizeof(unsigned char),1,fpOut);
            now_node=root;}
    }
    fclose(fpIn);
    fclose(fpOut);
}
//主函数进程
int main(){
    int n=0;
    int distance=0;
    int i,j;
    int *n_pointer=&n;
    int *dis_p=&distance;
    unsigned char*suffix=malloc(sizeof(unsigned char)*6);
    char*filename="testdir.scy";
    ascii_freq*frequency=head_data(filename,suffix,n_pointer,dis_p);//获取频度表
    huffT*HT=cre_huffT(frequency,n);//根据频度表创建Huffman树
    //创建新文件的名字
    char *cd=malloc(sizeof(char)*50);
    for(i=0;filename[i]!='.';i++)cd[i]=filename[i];
    cd[i++]='.';
    for(j=0;suffix[j]!='\0';j++)cd[j+i]=suffix[j];
    cd[j+i]='\0';
    char *newname=malloc(sizeof(char)*(i+j+1));
    strcpy(newname,cd);
    free(cd);
    //end
    decode(filename,newname,&HT[2*n-1-1],distance);//解压缩
    return 0;
}