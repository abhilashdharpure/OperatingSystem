
src/userspace/bin/init/init:     file format elf64-x86-64


Disassembly of section .text:

0000000040000000 <_start>:
    40000000:	48 31 ed             	xor    %rbp,%rbp
    40000003:	48 89 e7             	mov    %rsp,%rdi
    40000006:	48 8d 35 f3 ff ff bf 	lea    -0x4000000d(%rip),%rsi        # 0 <_start-0x40000000>
    4000000d:	48 83 e4 f0          	and    $0xfffffffffffffff0,%rsp
    40000011:	e8 0a 00 00 00       	call   40000020 <_start_c>
    40000016:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    4000001d:	00 00 00 

0000000040000020 <_start_c>:
    40000020:	8b 37                	mov    (%rdi),%esi
    40000022:	48 8d 57 08          	lea    0x8(%rdi),%rdx
    40000026:	45 31 c9             	xor    %r9d,%r9d
    40000029:	41 b8 67 61 00 40    	mov    $0x40006167,%r8d
    4000002f:	b9 64 61 00 40       	mov    $0x40006164,%ecx
    40000034:	bf 41 14 00 40       	mov    $0x40001441,%edi
    40000039:	e9 32 1c 00 00       	jmp    40001c70 <__libc_start_main>

000000004000003e <test_mmap>:
    4000003e:	55                   	push   %rbp
    4000003f:	48 89 e5             	mov    %rsp,%rbp
    40000042:	48 83 ec 20          	sub    $0x20,%rsp
    40000046:	48 c7 45 f0 00 10 00 	movq   $0x1000,-0x10(%rbp)
    4000004d:	00 
    4000004e:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    40000052:	41 b9 00 00 00 00    	mov    $0x0,%r9d
    40000058:	41 b8 ff ff ff ff    	mov    $0xffffffff,%r8d
    4000005e:	b9 22 00 00 00       	mov    $0x22,%ecx
    40000063:	ba 03 00 00 00       	mov    $0x3,%edx
    40000068:	48 89 c6             	mov    %rax,%rsi
    4000006b:	bf 00 00 00 00       	mov    $0x0,%edi
    40000070:	e8 6b 1f 00 00       	call   40001fe0 <__mmap>
    40000075:	48 89 45 e8          	mov    %rax,-0x18(%rbp)
    40000079:	48 83 7d e8 ff       	cmpq   $0xffffffffffffffff,-0x18(%rbp)
    4000007e:	75 14                	jne    40000094 <test_mmap+0x56>
    40000080:	bf 80 61 00 40       	mov    $0x40006180,%edi
    40000085:	b8 00 00 00 00       	mov    $0x0,%eax
    4000008a:	e8 b1 23 00 00       	call   40002440 <printf>
    4000008f:	e9 c0 00 00 00       	jmp    40000154 <test_mmap+0x116>
    40000094:	c7 45 fc 00 00 00 00 	movl   $0x0,-0x4(%rbp)
    4000009b:	eb 1f                	jmp    400000bc <test_mmap+0x7e>
    4000009d:	8b 45 fc             	mov    -0x4(%rbp),%eax
    400000a0:	48 63 d0             	movslq %eax,%rdx
    400000a3:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    400000a7:	48 01 c2             	add    %rax,%rdx
    400000aa:	8b 45 fc             	mov    -0x4(%rbp),%eax
    400000ad:	48 98                	cltq   
    400000af:	0f b6 80 d6 61 00 40 	movzbl 0x400061d6(%rax),%eax
    400000b6:	88 02                	mov    %al,(%rdx)
    400000b8:	83 45 fc 01          	addl   $0x1,-0x4(%rbp)
    400000bc:	83 7d fc 03          	cmpl   $0x3,-0x4(%rbp)
    400000c0:	7e db                	jle    4000009d <test_mmap+0x5f>
    400000c2:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    400000c6:	48 83 c0 04          	add    $0x4,%rax
    400000ca:	c6 00 00             	movb   $0x0,(%rax)
    400000cd:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    400000d1:	48 89 c6             	mov    %rax,%rsi
    400000d4:	bf 8d 61 00 40       	mov    $0x4000618d,%edi
    400000d9:	b8 00 00 00 00       	mov    $0x0,%eax
    400000de:	e8 5d 23 00 00       	call   40002440 <printf>
    400000e3:	48 8b 4d f0          	mov    -0x10(%rbp),%rcx
    400000e7:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    400000eb:	ba 01 00 00 00       	mov    $0x1,%edx
    400000f0:	48 89 ce             	mov    %rcx,%rsi
    400000f3:	48 89 c7             	mov    %rax,%rdi
    400000f6:	e8 c5 1f 00 00       	call   400020c0 <__mprotect>
    400000fb:	89 45 e4             	mov    %eax,-0x1c(%rbp)
    400000fe:	8b 45 e4             	mov    -0x1c(%rbp),%eax
    40000101:	89 c6                	mov    %eax,%esi
    40000103:	bf a0 61 00 40       	mov    $0x400061a0,%edi
    40000108:	b8 00 00 00 00       	mov    $0x0,%eax
    4000010d:	e8 2e 23 00 00       	call   40002440 <printf>
    40000112:	48 8b 4d f0          	mov    -0x10(%rbp),%rcx
    40000116:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    4000011a:	ba 03 00 00 00       	mov    $0x3,%edx
    4000011f:	48 89 ce             	mov    %rcx,%rsi
    40000122:	48 89 c7             	mov    %rax,%rdi
    40000125:	e8 96 1f 00 00       	call   400020c0 <__mprotect>
    4000012a:	89 45 e0             	mov    %eax,-0x20(%rbp)
    4000012d:	8b 45 e0             	mov    -0x20(%rbp),%eax
    40000130:	89 c6                	mov    %eax,%esi
    40000132:	bf bb 61 00 40       	mov    $0x400061bb,%edi
    40000137:	b8 00 00 00 00       	mov    $0x0,%eax
    4000013c:	e8 ff 22 00 00       	call   40002440 <printf>
    40000141:	48 8b 55 f0          	mov    -0x10(%rbp),%rdx
    40000145:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    40000149:	48 89 d6             	mov    %rdx,%rsi
    4000014c:	48 89 c7             	mov    %rax,%rdi
    4000014f:	e8 bc 1f 00 00       	call   40002110 <__munmap>
    40000154:	c9                   	leave  
    40000155:	c3                   	ret    

0000000040000156 <sbrk>:
    40000156:	55                   	push   %rbp
    40000157:	48 89 e5             	mov    %rsp,%rbp
    4000015a:	48 83 ec 30          	sub    $0x30,%rsp
    4000015e:	48 89 7d d8          	mov    %rdi,-0x28(%rbp)
    40000162:	48 83 ec 08          	sub    $0x8,%rsp
    40000166:	6a 00                	push   $0x0
    40000168:	41 b9 00 00 00 00    	mov    $0x0,%r9d
    4000016e:	41 b8 00 00 00 00    	mov    $0x0,%r8d
    40000174:	b9 00 00 00 00       	mov    $0x0,%ecx
    40000179:	ba 00 00 00 00       	mov    $0x0,%edx
    4000017e:	be 00 00 00 00       	mov    $0x0,%esi
    40000183:	bf 0c 00 00 00       	mov    $0xc,%edi
    40000188:	e8 0a 17 00 00       	call   40001897 <syscall6>
    4000018d:	48 83 c4 10          	add    $0x10,%rsp
    40000191:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    40000195:	48 83 7d f8 00       	cmpq   $0x0,-0x8(%rbp)
    4000019a:	75 09                	jne    400001a5 <sbrk+0x4f>
    4000019c:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    400001a3:	eb 5b                	jmp    40000200 <sbrk+0xaa>
    400001a5:	48 8b 55 f8          	mov    -0x8(%rbp),%rdx
    400001a9:	48 8b 45 d8          	mov    -0x28(%rbp),%rax
    400001ad:	48 01 d0             	add    %rdx,%rax
    400001b0:	48 89 45 f0          	mov    %rax,-0x10(%rbp)
    400001b4:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    400001b8:	48 83 ec 08          	sub    $0x8,%rsp
    400001bc:	6a 00                	push   $0x0
    400001be:	41 b9 00 00 00 00    	mov    $0x0,%r9d
    400001c4:	41 b8 00 00 00 00    	mov    $0x0,%r8d
    400001ca:	b9 00 00 00 00       	mov    $0x0,%ecx
    400001cf:	ba 00 00 00 00       	mov    $0x0,%edx
    400001d4:	48 89 c6             	mov    %rax,%rsi
    400001d7:	bf 0c 00 00 00       	mov    $0xc,%edi
    400001dc:	e8 b6 16 00 00       	call   40001897 <syscall6>
    400001e1:	48 83 c4 10          	add    $0x10,%rsp
    400001e5:	48 89 45 e8          	mov    %rax,-0x18(%rbp)
    400001e9:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    400001ed:	48 3b 45 f0          	cmp    -0x10(%rbp),%rax
    400001f1:	74 09                	je     400001fc <sbrk+0xa6>
    400001f3:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    400001fa:	eb 04                	jmp    40000200 <sbrk+0xaa>
    400001fc:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    40000200:	c9                   	leave  
    40000201:	c3                   	ret    

0000000040000202 <test_brk>:
    40000202:	55                   	push   %rbp
    40000203:	48 89 e5             	mov    %rsp,%rbp
    40000206:	48 83 ec 10          	sub    $0x10,%rsp
    4000020a:	bf db 61 00 40       	mov    $0x400061db,%edi
    4000020f:	b8 00 00 00 00       	mov    $0x0,%eax
    40000214:	e8 27 22 00 00       	call   40002440 <printf>
    40000219:	bf 00 00 00 00       	mov    $0x0,%edi
    4000021e:	e8 33 ff ff ff       	call   40000156 <sbrk>
    40000223:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    40000227:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    4000022b:	48 89 c6             	mov    %rax,%rsi
    4000022e:	bf ed 61 00 40       	mov    $0x400061ed,%edi
    40000233:	b8 00 00 00 00       	mov    $0x0,%eax
    40000238:	e8 03 22 00 00       	call   40002440 <printf>
    4000023d:	bf 00 10 00 00       	mov    $0x1000,%edi
    40000242:	e8 0f ff ff ff       	call   40000156 <sbrk>
    40000247:	48 89 45 f0          	mov    %rax,-0x10(%rbp)
    4000024b:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    4000024f:	48 89 c6             	mov    %rax,%rsi
    40000252:	bf 06 62 00 40       	mov    $0x40006206,%edi
    40000257:	b8 00 00 00 00       	mov    $0x0,%eax
    4000025c:	e8 df 21 00 00       	call   40002440 <printf>
    40000261:	90                   	nop
    40000262:	c9                   	leave  
    40000263:	c3                   	ret    

0000000040000264 <test_poll>:
    40000264:	55                   	push   %rbp
    40000265:	48 89 e5             	mov    %rsp,%rbp
    40000268:	48 83 ec 20          	sub    $0x20,%rsp
    4000026c:	89 7d ec             	mov    %edi,-0x14(%rbp)
    4000026f:	8b 45 ec             	mov    -0x14(%rbp),%eax
    40000272:	89 45 f4             	mov    %eax,-0xc(%rbp)
    40000275:	66 c7 45 f8 01 00    	movw   $0x1,-0x8(%rbp)
    4000027b:	66 c7 45 fa 00 00    	movw   $0x0,-0x6(%rbp)
    40000281:	48 8d 45 f4          	lea    -0xc(%rbp),%rax
    40000285:	ba 00 00 00 00       	mov    $0x0,%edx
    4000028a:	be 01 00 00 00       	mov    $0x1,%esi
    4000028f:	48 89 c7             	mov    %rax,%rdi
    40000292:	e8 59 21 00 00       	call   400023f0 <poll>
    40000297:	89 45 fc             	mov    %eax,-0x4(%rbp)
    4000029a:	0f b7 45 fa          	movzwl -0x6(%rbp),%eax
    4000029e:	0f bf d0             	movswl %ax,%edx
    400002a1:	8b 45 fc             	mov    -0x4(%rbp),%eax
    400002a4:	89 c6                	mov    %eax,%esi
    400002a6:	bf 28 62 00 40       	mov    $0x40006228,%edi
    400002ab:	b8 00 00 00 00       	mov    $0x0,%eax
    400002b0:	e8 8b 21 00 00       	call   40002440 <printf>
    400002b5:	90                   	nop
    400002b6:	c9                   	leave  
    400002b7:	c3                   	ret    

00000000400002b8 <print_ns>:
    400002b8:	55                   	push   %rbp
    400002b9:	48 89 e5             	mov    %rsp,%rbp
    400002bc:	48 83 ec 20          	sub    $0x20,%rsp
    400002c0:	89 7d ec             	mov    %edi,-0x14(%rbp)
    400002c3:	83 7d ec 00          	cmpl   $0x0,-0x14(%rbp)
    400002c7:	79 07                	jns    400002d0 <print_ns+0x18>
    400002c9:	81 45 ec 00 ca 9a 3b 	addl   $0x3b9aca00,-0x14(%rbp)
    400002d0:	c7 45 fc 09 00 00 00 	movl   $0x9,-0x4(%rbp)
    400002d7:	8b 45 ec             	mov    -0x14(%rbp),%eax
    400002da:	89 45 f8             	mov    %eax,-0x8(%rbp)
    400002dd:	eb 24                	jmp    40000303 <print_ns+0x4b>
    400002df:	8b 45 f8             	mov    -0x8(%rbp),%eax
    400002e2:	48 63 d0             	movslq %eax,%rdx
    400002e5:	48 69 d2 67 66 66 66 	imul   $0x66666667,%rdx,%rdx
    400002ec:	48 c1 ea 20          	shr    $0x20,%rdx
    400002f0:	c1 fa 02             	sar    $0x2,%edx
    400002f3:	c1 f8 1f             	sar    $0x1f,%eax
    400002f6:	89 c1                	mov    %eax,%ecx
    400002f8:	89 d0                	mov    %edx,%eax
    400002fa:	29 c8                	sub    %ecx,%eax
    400002fc:	89 45 f8             	mov    %eax,-0x8(%rbp)
    400002ff:	83 6d fc 01          	subl   $0x1,-0x4(%rbp)
    40000303:	83 7d f8 00          	cmpl   $0x0,-0x8(%rbp)
    40000307:	7f d6                	jg     400002df <print_ns+0x27>
    40000309:	eb 0f                	jmp    4000031a <print_ns+0x62>
    4000030b:	bf 48 62 00 40       	mov    $0x40006248,%edi
    40000310:	b8 00 00 00 00       	mov    $0x0,%eax
    40000315:	e8 26 21 00 00       	call   40002440 <printf>
    4000031a:	8b 45 fc             	mov    -0x4(%rbp),%eax
    4000031d:	8d 50 ff             	lea    -0x1(%rax),%edx
    40000320:	89 55 fc             	mov    %edx,-0x4(%rbp)
    40000323:	85 c0                	test   %eax,%eax
    40000325:	7f e4                	jg     4000030b <print_ns+0x53>
    40000327:	8b 45 ec             	mov    -0x14(%rbp),%eax
    4000032a:	89 c6                	mov    %eax,%esi
    4000032c:	bf 4a 62 00 40       	mov    $0x4000624a,%edi
    40000331:	b8 00 00 00 00       	mov    $0x0,%eax
    40000336:	e8 05 21 00 00       	call   40002440 <printf>
    4000033b:	90                   	nop
    4000033c:	c9                   	leave  
    4000033d:	c3                   	ret    

000000004000033e <test_socketpair_poll>:
    4000033e:	55                   	push   %rbp
    4000033f:	48 89 e5             	mov    %rsp,%rbp
    40000342:	48 83 ec 30          	sub    $0x30,%rsp
    40000346:	bf 4d 62 00 40       	mov    $0x4000624d,%edi
    4000034b:	b8 00 00 00 00       	mov    $0x0,%eax
    40000350:	e8 eb 20 00 00       	call   40002440 <printf>
    40000355:	48 8d 45 f0          	lea    -0x10(%rbp),%rax
    40000359:	48 89 c6             	mov    %rax,%rsi
    4000035c:	bf 6b 62 00 40       	mov    $0x4000626b,%edi
    40000361:	b8 00 00 00 00       	mov    $0x0,%eax
    40000366:	e8 d5 20 00 00       	call   40002440 <printf>
    4000036b:	48 8d 45 f0          	lea    -0x10(%rbp),%rax
    4000036f:	48 89 c1             	mov    %rax,%rcx
    40000372:	ba 00 00 00 00       	mov    $0x0,%edx
    40000377:	be 01 00 00 00       	mov    $0x1,%esi
    4000037c:	bf 01 00 00 00       	mov    $0x1,%edi
    40000381:	e8 6a 1f 00 00       	call   400022f0 <socketpair>
    40000386:	85 c0                	test   %eax,%eax
    40000388:	74 14                	je     4000039e <test_socketpair_poll+0x60>
    4000038a:	bf 75 62 00 40       	mov    $0x40006275,%edi
    4000038f:	b8 00 00 00 00       	mov    $0x0,%eax
    40000394:	e8 a7 20 00 00       	call   40002440 <printf>
    40000399:	e9 59 01 00 00       	jmp    400004f7 <test_socketpair_poll+0x1b9>
    4000039e:	8b 45 f0             	mov    -0x10(%rbp),%eax
    400003a1:	89 45 e0             	mov    %eax,-0x20(%rbp)
    400003a4:	66 c7 45 e4 05 00    	movw   $0x5,-0x1c(%rbp)
    400003aa:	8b 45 f4             	mov    -0xc(%rbp),%eax
    400003ad:	89 45 e8             	mov    %eax,-0x18(%rbp)
    400003b0:	66 c7 45 ec 05 00    	movw   $0x5,-0x14(%rbp)
    400003b6:	bf 88 62 00 40       	mov    $0x40006288,%edi
    400003bb:	b8 00 00 00 00       	mov    $0x0,%eax
    400003c0:	e8 7b 20 00 00       	call   40002440 <printf>
    400003c5:	48 8d 45 e0          	lea    -0x20(%rbp),%rax
    400003c9:	ba 00 00 00 00       	mov    $0x0,%edx
    400003ce:	be 02 00 00 00       	mov    $0x2,%esi
    400003d3:	48 89 c7             	mov    %rax,%rdi
    400003d6:	e8 15 20 00 00       	call   400023f0 <poll>
    400003db:	89 45 fc             	mov    %eax,-0x4(%rbp)
    400003de:	8b 45 fc             	mov    -0x4(%rbp),%eax
    400003e1:	89 c6                	mov    %eax,%esi
    400003e3:	bf a3 62 00 40       	mov    $0x400062a3,%edi
    400003e8:	b8 00 00 00 00       	mov    $0x0,%eax
    400003ed:	e8 4e 20 00 00       	call   40002440 <printf>
    400003f2:	0f b7 45 ee          	movzwl -0x12(%rbp),%eax
    400003f6:	0f bf d0             	movswl %ax,%edx
    400003f9:	0f b7 45 e6          	movzwl -0x1a(%rbp),%eax
    400003fd:	98                   	cwtl   
    400003fe:	89 c6                	mov    %eax,%esi
    40000400:	bf b8 62 00 40       	mov    $0x400062b8,%edi
    40000405:	b8 00 00 00 00       	mov    $0x0,%eax
    4000040a:	e8 31 20 00 00       	call   40002440 <printf>
    4000040f:	bf d8 62 00 40       	mov    $0x400062d8,%edi
    40000414:	b8 00 00 00 00       	mov    $0x0,%eax
    40000419:	e8 22 20 00 00       	call   40002440 <printf>
    4000041e:	8b 45 f0             	mov    -0x10(%rbp),%eax
    40000421:	ba 04 00 00 00       	mov    $0x4,%edx
    40000426:	be f8 62 00 40       	mov    $0x400062f8,%esi
    4000042b:	89 c7                	mov    %eax,%edi
    4000042d:	e8 ce 48 00 00       	call   40004d00 <write>
    40000432:	48 8d 45 e0          	lea    -0x20(%rbp),%rax
    40000436:	ba 10 00 00 00       	mov    $0x10,%edx
    4000043b:	be 00 00 00 00       	mov    $0x0,%esi
    40000440:	48 89 c7             	mov    %rax,%rdi
    40000443:	e8 97 45 00 00       	call   400049df <memset>
    40000448:	8b 45 f0             	mov    -0x10(%rbp),%eax
    4000044b:	89 45 e0             	mov    %eax,-0x20(%rbp)
    4000044e:	66 c7 45 e4 05 00    	movw   $0x5,-0x1c(%rbp)
    40000454:	8b 45 f4             	mov    -0xc(%rbp),%eax
    40000457:	89 45 e8             	mov    %eax,-0x18(%rbp)
    4000045a:	66 c7 45 ec 05 00    	movw   $0x5,-0x14(%rbp)
    40000460:	48 8d 45 e0          	lea    -0x20(%rbp),%rax
    40000464:	ba 00 00 00 00       	mov    $0x0,%edx
    40000469:	be 02 00 00 00       	mov    $0x2,%esi
    4000046e:	48 89 c7             	mov    %rax,%rdi
    40000471:	e8 7a 1f 00 00       	call   400023f0 <poll>
    40000476:	89 45 fc             	mov    %eax,-0x4(%rbp)
    40000479:	8b 45 fc             	mov    -0x4(%rbp),%eax
    4000047c:	89 c6                	mov    %eax,%esi
    4000047e:	bf 00 63 00 40       	mov    $0x40006300,%edi
    40000483:	b8 00 00 00 00       	mov    $0x0,%eax
    40000488:	e8 b3 1f 00 00       	call   40002440 <printf>
    4000048d:	0f b7 45 ee          	movzwl -0x12(%rbp),%eax
    40000491:	0f bf d0             	movswl %ax,%edx
    40000494:	0f b7 45 e6          	movzwl -0x1a(%rbp),%eax
    40000498:	98                   	cwtl   
    40000499:	89 c6                	mov    %eax,%esi
    4000049b:	bf b8 62 00 40       	mov    $0x400062b8,%edi
    400004a0:	b8 00 00 00 00       	mov    $0x0,%eax
    400004a5:	e8 96 1f 00 00       	call   40002440 <printf>
    400004aa:	48 c7 45 d8 00 00 00 	movq   $0x0,-0x28(%rbp)
    400004b1:	00 
    400004b2:	8b 45 f4             	mov    -0xc(%rbp),%eax
    400004b5:	48 8d 4d d8          	lea    -0x28(%rbp),%rcx
    400004b9:	ba 08 00 00 00       	mov    $0x8,%edx
    400004be:	48 89 ce             	mov    %rcx,%rsi
    400004c1:	89 c7                	mov    %eax,%edi
    400004c3:	e8 08 48 00 00       	call   40004cd0 <read>
    400004c8:	89 45 f8             	mov    %eax,-0x8(%rbp)
    400004cb:	48 8d 55 d8          	lea    -0x28(%rbp),%rdx
    400004cf:	8b 45 f8             	mov    -0x8(%rbp),%eax
    400004d2:	89 c6                	mov    %eax,%esi
    400004d4:	bf 20 63 00 40       	mov    $0x40006320,%edi
    400004d9:	b8 00 00 00 00       	mov    $0x0,%eax
    400004de:	e8 5d 1f 00 00       	call   40002440 <printf>
    400004e3:	8b 45 f0             	mov    -0x10(%rbp),%eax
    400004e6:	89 c7                	mov    %eax,%edi
    400004e8:	e8 f3 46 00 00       	call   40004be0 <close>
    400004ed:	8b 45 f4             	mov    -0xc(%rbp),%eax
    400004f0:	89 c7                	mov    %eax,%edi
    400004f2:	e8 e9 46 00 00       	call   40004be0 <close>
    400004f7:	c9                   	leave  
    400004f8:	c3                   	ret    

00000000400004f9 <test_socketpair_nonblock>:
    400004f9:	55                   	push   %rbp
    400004fa:	48 89 e5             	mov    %rsp,%rbp
    400004fd:	48 83 ec 20          	sub    $0x20,%rsp
    40000501:	bf 40 63 00 40       	mov    $0x40006340,%edi
    40000506:	b8 00 00 00 00       	mov    $0x0,%eax
    4000050b:	e8 30 1f 00 00       	call   40002440 <printf>
    40000510:	48 8d 45 f0          	lea    -0x10(%rbp),%rax
    40000514:	48 89 c1             	mov    %rax,%rcx
    40000517:	ba 00 00 00 00       	mov    $0x0,%edx
    4000051c:	be 01 00 00 00       	mov    $0x1,%esi
    40000521:	bf 01 00 00 00       	mov    $0x1,%edi
    40000526:	e8 c5 1d 00 00       	call   400022f0 <socketpair>
    4000052b:	85 c0                	test   %eax,%eax
    4000052d:	74 14                	je     40000543 <test_socketpair_nonblock+0x4a>
    4000052f:	bf 75 62 00 40       	mov    $0x40006275,%edi
    40000534:	b8 00 00 00 00       	mov    $0x0,%eax
    40000539:	e8 02 1f 00 00       	call   40002440 <printf>
    4000053e:	e9 c5 00 00 00       	jmp    40000608 <test_socketpair_nonblock+0x10f>
    40000543:	8b 45 f4             	mov    -0xc(%rbp),%eax
    40000546:	ba 00 00 00 00       	mov    $0x0,%edx
    4000054b:	be 03 00 00 00       	mov    $0x3,%esi
    40000550:	89 c7                	mov    %eax,%edi
    40000552:	e8 b9 17 00 00       	call   40001d10 <fcntl>
    40000557:	89 45 fc             	mov    %eax,-0x4(%rbp)
    4000055a:	8b 45 fc             	mov    -0x4(%rbp),%eax
    4000055d:	80 cc 04             	or     $0x4,%ah
    40000560:	48 63 d0             	movslq %eax,%rdx
    40000563:	8b 45 f4             	mov    -0xc(%rbp),%eax
    40000566:	be 04 00 00 00       	mov    $0x4,%esi
    4000056b:	89 c7                	mov    %eax,%edi
    4000056d:	e8 9e 17 00 00       	call   40001d10 <fcntl>
    40000572:	48 c7 45 e0 00 00 00 	movq   $0x0,-0x20(%rbp)
    40000579:	00 
    4000057a:	48 c7 45 e8 00 00 00 	movq   $0x0,-0x18(%rbp)
    40000581:	00 
    40000582:	8b 45 f4             	mov    -0xc(%rbp),%eax
    40000585:	48 8d 4d e0          	lea    -0x20(%rbp),%rcx
    40000589:	ba 10 00 00 00       	mov    $0x10,%edx
    4000058e:	48 89 ce             	mov    %rcx,%rsi
    40000591:	89 c7                	mov    %eax,%edi
    40000593:	e8 38 47 00 00       	call   40004cd0 <read>
    40000598:	89 45 f8             	mov    %eax,-0x8(%rbp)
    4000059b:	8b 45 f8             	mov    -0x8(%rbp),%eax
    4000059e:	89 c6                	mov    %eax,%esi
    400005a0:	bf 68 63 00 40       	mov    $0x40006368,%edi
    400005a5:	b8 00 00 00 00       	mov    $0x0,%eax
    400005aa:	e8 91 1e 00 00       	call   40002440 <printf>
    400005af:	8b 45 f0             	mov    -0x10(%rbp),%eax
    400005b2:	ba 01 00 00 00       	mov    $0x1,%edx
    400005b7:	be 89 63 00 40       	mov    $0x40006389,%esi
    400005bc:	89 c7                	mov    %eax,%edi
    400005be:	e8 3d 47 00 00       	call   40004d00 <write>
    400005c3:	8b 45 f4             	mov    -0xc(%rbp),%eax
    400005c6:	48 8d 4d e0          	lea    -0x20(%rbp),%rcx
    400005ca:	ba 10 00 00 00       	mov    $0x10,%edx
    400005cf:	48 89 ce             	mov    %rcx,%rsi
    400005d2:	89 c7                	mov    %eax,%edi
    400005d4:	e8 f7 46 00 00       	call   40004cd0 <read>
    400005d9:	89 45 f8             	mov    %eax,-0x8(%rbp)
    400005dc:	48 8d 55 e0          	lea    -0x20(%rbp),%rdx
    400005e0:	8b 45 f8             	mov    -0x8(%rbp),%eax
    400005e3:	89 c6                	mov    %eax,%esi
    400005e5:	bf 90 63 00 40       	mov    $0x40006390,%edi
    400005ea:	b8 00 00 00 00       	mov    $0x0,%eax
    400005ef:	e8 4c 1e 00 00       	call   40002440 <printf>
    400005f4:	8b 45 f0             	mov    -0x10(%rbp),%eax
    400005f7:	89 c7                	mov    %eax,%edi
    400005f9:	e8 e2 45 00 00       	call   40004be0 <close>
    400005fe:	8b 45 f4             	mov    -0xc(%rbp),%eax
    40000601:	89 c7                	mov    %eax,%edi
    40000603:	e8 d8 45 00 00       	call   40004be0 <close>
    40000608:	c9                   	leave  
    40000609:	c3                   	ret    

000000004000060a <test_syscalls_presence>:
    4000060a:	55                   	push   %rbp
    4000060b:	48 89 e5             	mov    %rsp,%rbp
    4000060e:	48 83 ec 60          	sub    $0x60,%rsp
    40000612:	48 c7 45 b0 00 00 00 	movq   $0x0,-0x50(%rbp)
    40000619:	00 
    4000061a:	48 c7 45 b8 00 00 00 	movq   $0x0,-0x48(%rbp)
    40000621:	00 
    40000622:	48 c7 45 c0 00 00 00 	movq   $0x0,-0x40(%rbp)
    40000629:	00 
    4000062a:	48 c7 45 c8 00 00 00 	movq   $0x0,-0x38(%rbp)
    40000631:	00 
    40000632:	48 c7 45 d0 00 00 00 	movq   $0x0,-0x30(%rbp)
    40000639:	00 
    4000063a:	48 c7 45 d8 00 00 00 	movq   $0x0,-0x28(%rbp)
    40000641:	00 
    40000642:	48 c7 45 e0 00 00 00 	movq   $0x0,-0x20(%rbp)
    40000649:	00 
    4000064a:	48 8d 45 a8          	lea    -0x58(%rbp),%rax
    4000064e:	48 89 c1             	mov    %rax,%rcx
    40000651:	ba 00 00 00 00       	mov    $0x0,%edx
    40000656:	be 01 00 00 00       	mov    $0x1,%esi
    4000065b:	bf 01 00 00 00       	mov    $0x1,%edi
    40000660:	e8 8b 1c 00 00       	call   400022f0 <socketpair>
    40000665:	85 c0                	test   %eax,%eax
    40000667:	75 5e                	jne    400006c7 <test_syscalls_presence+0xbd>
    40000669:	8b 45 a8             	mov    -0x58(%rbp),%eax
    4000066c:	48 8d 4d b0          	lea    -0x50(%rbp),%rcx
    40000670:	ba 00 00 00 00       	mov    $0x0,%edx
    40000675:	48 89 ce             	mov    %rcx,%rsi
    40000678:	89 c7                	mov    %eax,%edi
    4000067a:	e8 61 1b 00 00       	call   400021e0 <sendmsg>
    4000067f:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    40000683:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    40000687:	89 c6                	mov    %eax,%esi
    40000689:	bf bd 63 00 40       	mov    $0x400063bd,%edi
    4000068e:	b8 00 00 00 00       	mov    $0x0,%eax
    40000693:	e8 a8 1d 00 00       	call   40002440 <printf>
    40000698:	8b 45 ac             	mov    -0x54(%rbp),%eax
    4000069b:	48 8d 4d b0          	lea    -0x50(%rbp),%rcx
    4000069f:	ba 00 00 00 00       	mov    $0x0,%edx
    400006a4:	48 89 ce             	mov    %rcx,%rsi
    400006a7:	89 c7                	mov    %eax,%edi
    400006a9:	e8 a2 1a 00 00       	call   40002150 <recvmsg>
    400006ae:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    400006b2:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    400006b6:	89 c6                	mov    %eax,%esi
    400006b8:	bf d2 63 00 40       	mov    $0x400063d2,%edi
    400006bd:	b8 00 00 00 00       	mov    $0x0,%eax
    400006c2:	e8 79 1d 00 00       	call   40002440 <printf>
    400006c7:	be 00 00 00 00       	mov    $0x0,%esi
    400006cc:	bf e7 63 00 40       	mov    $0x400063e7,%edi
    400006d1:	e8 da 18 00 00       	call   40001fb0 <memfd_create>
    400006d6:	89 45 f4             	mov    %eax,-0xc(%rbp)
    400006d9:	8b 45 f4             	mov    -0xc(%rbp),%eax
    400006dc:	89 c6                	mov    %eax,%esi
    400006de:	bf ec 63 00 40       	mov    $0x400063ec,%edi
    400006e3:	b8 00 00 00 00       	mov    $0x0,%eax
    400006e8:	e8 53 1d 00 00       	call   40002440 <printf>
    400006ed:	90                   	nop
    400006ee:	c9                   	leave  
    400006ef:	c3                   	ret    

00000000400006f0 <test_memfd>:
    400006f0:	55                   	push   %rbp
    400006f1:	48 89 e5             	mov    %rsp,%rbp
    400006f4:	48 83 ec 30          	sub    $0x30,%rsp
    400006f8:	bf 06 64 00 40       	mov    $0x40006406,%edi
    400006fd:	b8 00 00 00 00       	mov    $0x0,%eax
    40000702:	e8 39 1d 00 00       	call   40002440 <printf>
    40000707:	be 00 00 00 00       	mov    $0x0,%esi
    4000070c:	bf e7 63 00 40       	mov    $0x400063e7,%edi
    40000711:	e8 9a 18 00 00       	call   40001fb0 <memfd_create>
    40000716:	89 45 fc             	mov    %eax,-0x4(%rbp)
    40000719:	8b 45 fc             	mov    -0x4(%rbp),%eax
    4000071c:	89 c6                	mov    %eax,%esi
    4000071e:	bf ec 63 00 40       	mov    $0x400063ec,%edi
    40000723:	b8 00 00 00 00       	mov    $0x0,%eax
    40000728:	e8 13 1d 00 00       	call   40002440 <printf>
    4000072d:	83 7d fc 00          	cmpl   $0x0,-0x4(%rbp)
    40000731:	78 7d                	js     400007b0 <test_memfd+0xc0>
    40000733:	48 c7 45 f0 1f 64 00 	movq   $0x4000641f,-0x10(%rbp)
    4000073a:	40 
    4000073b:	48 8b 4d f0          	mov    -0x10(%rbp),%rcx
    4000073f:	8b 45 fc             	mov    -0x4(%rbp),%eax
    40000742:	ba 0b 00 00 00       	mov    $0xb,%edx
    40000747:	48 89 ce             	mov    %rcx,%rsi
    4000074a:	89 c7                	mov    %eax,%edi
    4000074c:	e8 af 45 00 00       	call   40004d00 <write>
    40000751:	8b 45 fc             	mov    -0x4(%rbp),%eax
    40000754:	ba 00 00 00 00       	mov    $0x0,%edx
    40000759:	be 00 00 00 00       	mov    $0x0,%esi
    4000075e:	89 c7                	mov    %eax,%edi
    40000760:	e8 2b 45 00 00       	call   40004c90 <__lseek>
    40000765:	48 c7 45 d0 00 00 00 	movq   $0x0,-0x30(%rbp)
    4000076c:	00 
    4000076d:	48 c7 45 d8 00 00 00 	movq   $0x0,-0x28(%rbp)
    40000774:	00 
    40000775:	48 8d 4d d0          	lea    -0x30(%rbp),%rcx
    40000779:	8b 45 fc             	mov    -0x4(%rbp),%eax
    4000077c:	ba 10 00 00 00       	mov    $0x10,%edx
    40000781:	48 89 ce             	mov    %rcx,%rsi
    40000784:	89 c7                	mov    %eax,%edi
    40000786:	e8 45 45 00 00       	call   40004cd0 <read>
    4000078b:	89 45 ec             	mov    %eax,-0x14(%rbp)
    4000078e:	48 8d 55 d0          	lea    -0x30(%rbp),%rdx
    40000792:	8b 45 ec             	mov    -0x14(%rbp),%eax
    40000795:	89 c6                	mov    %eax,%esi
    40000797:	bf 30 64 00 40       	mov    $0x40006430,%edi
    4000079c:	b8 00 00 00 00       	mov    $0x0,%eax
    400007a1:	e8 9a 1c 00 00       	call   40002440 <printf>
    400007a6:	8b 45 fc             	mov    -0x4(%rbp),%eax
    400007a9:	89 c7                	mov    %eax,%edi
    400007ab:	e8 30 44 00 00       	call   40004be0 <close>
    400007b0:	90                   	nop
    400007b1:	c9                   	leave  
    400007b2:	c3                   	ret    

00000000400007b3 <test_ftruncate_memfd>:
    400007b3:	55                   	push   %rbp
    400007b4:	48 89 e5             	mov    %rsp,%rbp
    400007b7:	48 83 ec 20          	sub    $0x20,%rsp
    400007bb:	bf 58 64 00 40       	mov    $0x40006458,%edi
    400007c0:	b8 00 00 00 00       	mov    $0x0,%eax
    400007c5:	e8 76 1c 00 00       	call   40002440 <printf>
    400007ca:	be 00 00 00 00       	mov    $0x0,%esi
    400007cf:	bf 77 64 00 40       	mov    $0x40006477,%edi
    400007d4:	e8 d7 17 00 00       	call   40001fb0 <memfd_create>
    400007d9:	89 45 fc             	mov    %eax,-0x4(%rbp)
    400007dc:	8b 45 fc             	mov    -0x4(%rbp),%eax
    400007df:	89 c6                	mov    %eax,%esi
    400007e1:	bf 7c 64 00 40       	mov    $0x4000647c,%edi
    400007e6:	b8 00 00 00 00       	mov    $0x0,%eax
    400007eb:	e8 50 1c 00 00       	call   40002440 <printf>
    400007f0:	8b 45 fc             	mov    -0x4(%rbp),%eax
    400007f3:	be 00 10 00 00       	mov    $0x1000,%esi
    400007f8:	89 c7                	mov    %eax,%edi
    400007fa:	e8 71 44 00 00       	call   40004c70 <ftruncate>
    400007ff:	8b 45 fc             	mov    -0x4(%rbp),%eax
    40000802:	ba 00 00 00 00       	mov    $0x0,%edx
    40000807:	be 00 00 00 00       	mov    $0x0,%esi
    4000080c:	89 c7                	mov    %eax,%edi
    4000080e:	e8 7d 44 00 00       	call   40004c90 <__lseek>
    40000813:	48 c7 45 e0 00 00 00 	movq   $0x0,-0x20(%rbp)
    4000081a:	00 
    4000081b:	48 c7 45 e8 00 00 00 	movq   $0x0,-0x18(%rbp)
    40000822:	00 
    40000823:	48 8d 4d e0          	lea    -0x20(%rbp),%rcx
    40000827:	8b 45 fc             	mov    -0x4(%rbp),%eax
    4000082a:	ba 10 00 00 00       	mov    $0x10,%edx
    4000082f:	48 89 ce             	mov    %rcx,%rsi
    40000832:	89 c7                	mov    %eax,%edi
    40000834:	e8 97 44 00 00       	call   40004cd0 <read>
    40000839:	89 45 f8             	mov    %eax,-0x8(%rbp)
    4000083c:	8b 45 f8             	mov    -0x8(%rbp),%eax
    4000083f:	89 c6                	mov    %eax,%esi
    40000841:	bf 90 64 00 40       	mov    $0x40006490,%edi
    40000846:	b8 00 00 00 00       	mov    $0x0,%eax
    4000084b:	e8 f0 1b 00 00       	call   40002440 <printf>
    40000850:	8b 45 fc             	mov    -0x4(%rbp),%eax
    40000853:	89 c7                	mov    %eax,%edi
    40000855:	e8 86 43 00 00       	call   40004be0 <close>
    4000085a:	90                   	nop
    4000085b:	c9                   	leave  
    4000085c:	c3                   	ret    

000000004000085d <test_memfd_mmap>:
    4000085d:	55                   	push   %rbp
    4000085e:	48 89 e5             	mov    %rsp,%rbp
    40000861:	48 83 ec 30          	sub    $0x30,%rsp
    40000865:	bf b1 64 00 40       	mov    $0x400064b1,%edi
    4000086a:	b8 00 00 00 00       	mov    $0x0,%eax
    4000086f:	e8 cc 1b 00 00       	call   40002440 <printf>
    40000874:	be 00 00 00 00       	mov    $0x0,%esi
    40000879:	bf ca 64 00 40       	mov    $0x400064ca,%edi
    4000087e:	e8 2d 17 00 00       	call   40001fb0 <memfd_create>
    40000883:	89 45 fc             	mov    %eax,-0x4(%rbp)
    40000886:	8b 45 fc             	mov    -0x4(%rbp),%eax
    40000889:	89 c6                	mov    %eax,%esi
    4000088b:	bf 7c 64 00 40       	mov    $0x4000647c,%edi
    40000890:	b8 00 00 00 00       	mov    $0x0,%eax
    40000895:	e8 a6 1b 00 00       	call   40002440 <printf>
    4000089a:	48 c7 45 f0 00 10 00 	movq   $0x1000,-0x10(%rbp)
    400008a1:	00 
    400008a2:	48 8b 55 f0          	mov    -0x10(%rbp),%rdx
    400008a6:	8b 45 fc             	mov    -0x4(%rbp),%eax
    400008a9:	48 89 d6             	mov    %rdx,%rsi
    400008ac:	89 c7                	mov    %eax,%edi
    400008ae:	e8 bd 43 00 00       	call   40004c70 <ftruncate>
    400008b3:	bf d7 64 00 40       	mov    $0x400064d7,%edi
    400008b8:	b8 00 00 00 00       	mov    $0x0,%eax
    400008bd:	e8 7e 1b 00 00       	call   40002440 <printf>
    400008c2:	8b 55 fc             	mov    -0x4(%rbp),%edx
    400008c5:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    400008c9:	41 b9 00 00 00 00    	mov    $0x0,%r9d
    400008cf:	41 89 d0             	mov    %edx,%r8d
    400008d2:	b9 01 00 00 00       	mov    $0x1,%ecx
    400008d7:	ba 03 00 00 00       	mov    $0x3,%edx
    400008dc:	48 89 c6             	mov    %rax,%rsi
    400008df:	bf 00 00 00 00       	mov    $0x0,%edi
    400008e4:	e8 f7 16 00 00       	call   40001fe0 <__mmap>
    400008e9:	48 89 45 e8          	mov    %rax,-0x18(%rbp)
    400008ed:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    400008f1:	48 89 c6             	mov    %rax,%rsi
    400008f4:	bf ec 64 00 40       	mov    $0x400064ec,%edi
    400008f9:	b8 00 00 00 00       	mov    $0x0,%eax
    400008fe:	e8 3d 1b 00 00       	call   40002440 <printf>
    40000903:	48 83 7d e8 ff       	cmpq   $0xffffffffffffffff,-0x18(%rbp)
    40000908:	75 1e                	jne    40000928 <test_memfd_mmap+0xcb>
    4000090a:	bf 80 61 00 40       	mov    $0x40006180,%edi
    4000090f:	b8 00 00 00 00       	mov    $0x0,%eax
    40000914:	e8 27 1b 00 00       	call   40002440 <printf>
    40000919:	8b 45 fc             	mov    -0x4(%rbp),%eax
    4000091c:	89 c7                	mov    %eax,%edi
    4000091e:	e8 bd 42 00 00       	call   40004be0 <close>
    40000923:	e9 9d 00 00 00       	jmp    400009c5 <test_memfd_mmap+0x168>
    40000928:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    4000092c:	48 89 45 e0          	mov    %rax,-0x20(%rbp)
    40000930:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
    40000934:	c6 00 5a             	movb   $0x5a,(%rax)
    40000937:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
    4000093b:	48 83 c0 01          	add    $0x1,%rax
    4000093f:	c6 00 59             	movb   $0x59,(%rax)
    40000942:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
    40000946:	48 83 c0 02          	add    $0x2,%rax
    4000094a:	c6 00 58             	movb   $0x58,(%rax)
    4000094d:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
    40000951:	48 83 c0 03          	add    $0x3,%rax
    40000955:	c6 00 00             	movb   $0x0,(%rax)
    40000958:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
    4000095c:	48 89 c6             	mov    %rax,%rsi
    4000095f:	bf fe 64 00 40       	mov    $0x400064fe,%edi
    40000964:	b8 00 00 00 00       	mov    $0x0,%eax
    40000969:	e8 d2 1a 00 00       	call   40002440 <printf>
    4000096e:	8b 45 fc             	mov    -0x4(%rbp),%eax
    40000971:	ba 00 00 00 00       	mov    $0x0,%edx
    40000976:	be 00 00 00 00       	mov    $0x0,%esi
    4000097b:	89 c7                	mov    %eax,%edi
    4000097d:	e8 0e 43 00 00       	call   40004c90 <__lseek>
    40000982:	48 c7 45 d4 00 00 00 	movq   $0x0,-0x2c(%rbp)
    40000989:	00 
    4000098a:	48 8d 4d d4          	lea    -0x2c(%rbp),%rcx
    4000098e:	8b 45 fc             	mov    -0x4(%rbp),%eax
    40000991:	ba 08 00 00 00       	mov    $0x8,%edx
    40000996:	48 89 ce             	mov    %rcx,%rsi
    40000999:	89 c7                	mov    %eax,%edi
    4000099b:	e8 30 43 00 00       	call   40004cd0 <read>
    400009a0:	89 45 dc             	mov    %eax,-0x24(%rbp)
    400009a3:	48 8d 55 d4          	lea    -0x2c(%rbp),%rdx
    400009a7:	8b 45 dc             	mov    -0x24(%rbp),%eax
    400009aa:	89 c6                	mov    %eax,%esi
    400009ac:	bf 18 65 00 40       	mov    $0x40006518,%edi
    400009b1:	b8 00 00 00 00       	mov    $0x0,%eax
    400009b6:	e8 85 1a 00 00       	call   40002440 <printf>
    400009bb:	8b 45 fc             	mov    -0x4(%rbp),%eax
    400009be:	89 c7                	mov    %eax,%edi
    400009c0:	e8 1b 42 00 00       	call   40004be0 <close>
    400009c5:	c9                   	leave  
    400009c6:	c3                   	ret    

00000000400009c7 <test_raw_socketpair>:
    400009c7:	55                   	push   %rbp
    400009c8:	48 89 e5             	mov    %rsp,%rbp
    400009cb:	48 83 ec 10          	sub    $0x10,%rsp
    400009cf:	48 8d 45 f0          	lea    -0x10(%rbp),%rax
    400009d3:	48 89 c6             	mov    %rax,%rsi
    400009d6:	bf 3f 65 00 40       	mov    $0x4000653f,%edi
    400009db:	b8 00 00 00 00       	mov    $0x0,%eax
    400009e0:	e8 5b 1a 00 00       	call   40002440 <printf>
    400009e5:	48 8d 45 f0          	lea    -0x10(%rbp),%rax
    400009e9:	48 83 ec 08          	sub    $0x8,%rsp
    400009ed:	6a 00                	push   $0x0
    400009ef:	41 b9 00 00 00 00    	mov    $0x0,%r9d
    400009f5:	49 89 c0             	mov    %rax,%r8
    400009f8:	b9 00 00 00 00       	mov    $0x0,%ecx
    400009fd:	ba 01 00 00 00       	mov    $0x1,%edx
    40000a02:	be 01 00 00 00       	mov    $0x1,%esi
    40000a07:	bf 35 00 00 00       	mov    $0x35,%edi
    40000a0c:	e8 86 0e 00 00       	call   40001897 <syscall6>
    40000a11:	48 83 c4 10          	add    $0x10,%rsp
    40000a15:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    40000a19:	8b 4d f4             	mov    -0xc(%rbp),%ecx
    40000a1c:	8b 55 f0             	mov    -0x10(%rbp),%edx
    40000a1f:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    40000a23:	48 89 c6             	mov    %rax,%rsi
    40000a26:	bf 50 65 00 40       	mov    $0x40006550,%edi
    40000a2b:	b8 00 00 00 00       	mov    $0x0,%eax
    40000a30:	e8 0b 1a 00 00       	call   40002440 <printf>
    40000a35:	90                   	nop
    40000a36:	c9                   	leave  
    40000a37:	c3                   	ret    

0000000040000a38 <testAllSyscalls>:
    40000a38:	55                   	push   %rbp
    40000a39:	48 89 e5             	mov    %rsp,%rbp
    40000a3c:	48 81 ec 40 08 00 00 	sub    $0x840,%rsp
    40000a43:	bf 80 65 00 40       	mov    $0x40006580,%edi
    40000a48:	b8 00 00 00 00       	mov    $0x0,%eax
    40000a4d:	e8 ee 19 00 00       	call   40002440 <printf>
    40000a52:	ba 00 00 00 00       	mov    $0x0,%edx
    40000a57:	be 00 00 00 00       	mov    $0x0,%esi
    40000a5c:	bf ab 65 00 40       	mov    $0x400065ab,%edi
    40000a61:	e8 3a 14 00 00       	call   40001ea0 <open>
    40000a66:	89 45 f8             	mov    %eax,-0x8(%rbp)
    40000a69:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000a6c:	89 c6                	mov    %eax,%esi
    40000a6e:	bf bc 65 00 40       	mov    $0x400065bc,%edi
    40000a73:	b8 00 00 00 00       	mov    $0x0,%eax
    40000a78:	e8 c3 19 00 00       	call   40002440 <printf>
    40000a7d:	83 7d f8 00          	cmpl   $0x0,-0x8(%rbp)
    40000a81:	0f 88 84 00 00 00    	js     40000b0b <testAllSyscalls+0xd3>
    40000a87:	48 8d 8d c0 f7 ff ff 	lea    -0x840(%rbp),%rcx
    40000a8e:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000a91:	ba 7f 00 00 00       	mov    $0x7f,%edx
    40000a96:	48 89 ce             	mov    %rcx,%rsi
    40000a99:	89 c7                	mov    %eax,%edi
    40000a9b:	e8 30 42 00 00       	call   40004cd0 <read>
    40000aa0:	48 89 45 f0          	mov    %rax,-0x10(%rbp)
    40000aa4:	48 83 7d f0 00       	cmpq   $0x0,-0x10(%rbp)
    40000aa9:	7e 41                	jle    40000aec <testAllSyscalls+0xb4>
    40000aab:	48 8d 95 c0 f7 ff ff 	lea    -0x840(%rbp),%rdx
    40000ab2:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    40000ab6:	48 01 d0             	add    %rdx,%rax
    40000ab9:	c6 00 00             	movb   $0x0,(%rax)
    40000abc:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    40000ac0:	89 c6                	mov    %eax,%esi
    40000ac2:	bf d1 65 00 40       	mov    $0x400065d1,%edi
    40000ac7:	b8 00 00 00 00       	mov    $0x0,%eax
    40000acc:	e8 6f 19 00 00       	call   40002440 <printf>
    40000ad1:	48 8d 85 c0 f7 ff ff 	lea    -0x840(%rbp),%rax
    40000ad8:	48 89 c6             	mov    %rax,%rsi
    40000adb:	bf e9 65 00 40       	mov    $0x400065e9,%edi
    40000ae0:	b8 00 00 00 00       	mov    $0x0,%eax
    40000ae5:	e8 56 19 00 00       	call   40002440 <printf>
    40000aea:	eb 15                	jmp    40000b01 <testAllSyscalls+0xc9>
    40000aec:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
    40000af0:	89 c6                	mov    %eax,%esi
    40000af2:	bf f7 65 00 40       	mov    $0x400065f7,%edi
    40000af7:	b8 00 00 00 00       	mov    $0x0,%eax
    40000afc:	e8 3f 19 00 00       	call   40002440 <printf>
    40000b01:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000b04:	89 c7                	mov    %eax,%edi
    40000b06:	e8 d5 40 00 00       	call   40004be0 <close>
    40000b0b:	bf 10 66 00 40       	mov    $0x40006610,%edi
    40000b10:	b8 00 00 00 00       	mov    $0x0,%eax
    40000b15:	e8 26 19 00 00       	call   40002440 <printf>
    40000b1a:	e8 1f f5 ff ff       	call   4000003e <test_mmap>
    40000b1f:	bf 21 66 00 40       	mov    $0x40006621,%edi
    40000b24:	b8 00 00 00 00       	mov    $0x0,%eax
    40000b29:	e8 12 19 00 00       	call   40002440 <printf>
    40000b2e:	e8 cf f6 ff ff       	call   40000202 <test_brk>
    40000b33:	bf 31 66 00 40       	mov    $0x40006631,%edi
    40000b38:	b8 00 00 00 00       	mov    $0x0,%eax
    40000b3d:	e8 fe 18 00 00       	call   40002440 <printf>
    40000b42:	ba 00 00 00 00       	mov    $0x0,%edx
    40000b47:	be 00 00 00 00       	mov    $0x0,%esi
    40000b4c:	bf ab 65 00 40       	mov    $0x400065ab,%edi
    40000b51:	e8 4a 13 00 00       	call   40001ea0 <open>
    40000b56:	89 45 f8             	mov    %eax,-0x8(%rbp)
    40000b59:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000b5c:	89 c6                	mov    %eax,%esi
    40000b5e:	bf 42 66 00 40       	mov    $0x40006642,%edi
    40000b63:	b8 00 00 00 00       	mov    $0x0,%eax
    40000b68:	e8 d3 18 00 00       	call   40002440 <printf>
    40000b6d:	83 7d f8 00          	cmpl   $0x0,-0x8(%rbp)
    40000b71:	78 46                	js     40000bb9 <testAllSyscalls+0x181>
    40000b73:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000b76:	89 c7                	mov    %eax,%edi
    40000b78:	e8 e7 f6 ff ff       	call   40000264 <test_poll>
    40000b7d:	48 8d 8d c0 f7 ff ff 	lea    -0x840(%rbp),%rcx
    40000b84:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000b87:	ba 7f 00 00 00       	mov    $0x7f,%edx
    40000b8c:	48 89 ce             	mov    %rcx,%rsi
    40000b8f:	89 c7                	mov    %eax,%edi
    40000b91:	e8 3a 41 00 00       	call   40004cd0 <read>
    40000b96:	48 89 45 e8          	mov    %rax,-0x18(%rbp)
    40000b9a:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    40000b9e:	89 c6                	mov    %eax,%esi
    40000ba0:	bf 5c 66 00 40       	mov    $0x4000665c,%edi
    40000ba5:	b8 00 00 00 00       	mov    $0x0,%eax
    40000baa:	e8 91 18 00 00       	call   40002440 <printf>
    40000baf:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000bb2:	89 c7                	mov    %eax,%edi
    40000bb4:	e8 27 40 00 00       	call   40004be0 <close>
    40000bb9:	bf 79 66 00 40       	mov    $0x40006679,%edi
    40000bbe:	b8 00 00 00 00       	mov    $0x0,%eax
    40000bc3:	e8 78 18 00 00       	call   40002440 <printf>
    40000bc8:	48 8d 85 70 ff ff ff 	lea    -0x90(%rbp),%rax
    40000bcf:	48 89 c6             	mov    %rax,%rsi
    40000bd2:	bf ab 65 00 40       	mov    $0x400065ab,%edi
    40000bd7:	e8 44 18 00 00       	call   40002420 <stat>
    40000bdc:	85 c0                	test   %eax,%eax
    40000bde:	75 20                	jne    40000c00 <testAllSyscalls+0x1c8>
    40000be0:	48 8b 85 78 ff ff ff 	mov    -0x88(%rbp),%rax
    40000be7:	89 c2                	mov    %eax,%edx
    40000be9:	48 8b 45 88          	mov    -0x78(%rbp),%rax
    40000bed:	89 c6                	mov    %eax,%esi
    40000bef:	bf 8a 66 00 40       	mov    $0x4000668a,%edi
    40000bf4:	b8 00 00 00 00       	mov    $0x0,%eax
    40000bf9:	e8 42 18 00 00       	call   40002440 <printf>
    40000bfe:	eb 0f                	jmp    40000c0f <testAllSyscalls+0x1d7>
    40000c00:	bf a2 66 00 40       	mov    $0x400066a2,%edi
    40000c05:	b8 00 00 00 00       	mov    $0x0,%eax
    40000c0a:	e8 31 18 00 00       	call   40002440 <printf>
    40000c0f:	bf af 66 00 40       	mov    $0x400066af,%edi
    40000c14:	b8 00 00 00 00       	mov    $0x0,%eax
    40000c19:	e8 22 18 00 00       	call   40002440 <printf>
    40000c1e:	ba 00 00 00 00       	mov    $0x0,%edx
    40000c23:	be 00 00 00 00       	mov    $0x0,%esi
    40000c28:	bf ab 65 00 40       	mov    $0x400065ab,%edi
    40000c2d:	e8 6e 12 00 00       	call   40001ea0 <open>
    40000c32:	89 45 e4             	mov    %eax,-0x1c(%rbp)
    40000c35:	8b 45 e4             	mov    -0x1c(%rbp),%eax
    40000c38:	89 c6                	mov    %eax,%esi
    40000c3a:	bf c1 66 00 40       	mov    $0x400066c1,%edi
    40000c3f:	b8 00 00 00 00       	mov    $0x0,%eax
    40000c44:	e8 f7 17 00 00       	call   40002440 <printf>
    40000c49:	83 7d e4 00          	cmpl   $0x0,-0x1c(%rbp)
    40000c4d:	0f 88 81 00 00 00    	js     40000cd4 <testAllSyscalls+0x29c>
    40000c53:	48 c7 85 40 f8 ff ff 	movq   $0x0,-0x7c0(%rbp)
    40000c5a:	00 00 00 00 
    40000c5e:	8b 45 e4             	mov    -0x1c(%rbp),%eax
    40000c61:	ba 00 00 00 00       	mov    $0x0,%edx
    40000c66:	be 02 00 00 00       	mov    $0x2,%esi
    40000c6b:	89 c7                	mov    %eax,%edi
    40000c6d:	e8 1e 40 00 00       	call   40004c90 <__lseek>
    40000c72:	48 89 45 d8          	mov    %rax,-0x28(%rbp)
    40000c76:	48 8b 45 d8          	mov    -0x28(%rbp),%rax
    40000c7a:	48 89 c6             	mov    %rax,%rsi
    40000c7d:	bf dc 66 00 40       	mov    $0x400066dc,%edi
    40000c82:	b8 00 00 00 00       	mov    $0x0,%eax
    40000c87:	e8 b4 17 00 00       	call   40002440 <printf>
    40000c8c:	48 8d 8d 40 f8 ff ff 	lea    -0x7c0(%rbp),%rcx
    40000c93:	8b 45 e4             	mov    -0x1c(%rbp),%eax
    40000c96:	ba 04 00 00 00       	mov    $0x4,%edx
    40000c9b:	48 89 ce             	mov    %rcx,%rsi
    40000c9e:	89 c7                	mov    %eax,%edi
    40000ca0:	e8 2b 40 00 00       	call   40004cd0 <read>
    40000ca5:	48 89 45 d0          	mov    %rax,-0x30(%rbp)
    40000ca9:	48 8b 45 d0          	mov    -0x30(%rbp),%rax
    40000cad:	89 c1                	mov    %eax,%ecx
    40000caf:	48 8d 85 40 f8 ff ff 	lea    -0x7c0(%rbp),%rax
    40000cb6:	48 89 c2             	mov    %rax,%rdx
    40000cb9:	89 ce                	mov    %ecx,%esi
    40000cbb:	bf f0 66 00 40       	mov    $0x400066f0,%edi
    40000cc0:	b8 00 00 00 00       	mov    $0x0,%eax
    40000cc5:	e8 76 17 00 00       	call   40002440 <printf>
    40000cca:	8b 45 e4             	mov    -0x1c(%rbp),%eax
    40000ccd:	89 c7                	mov    %eax,%edi
    40000ccf:	e8 0c 3f 00 00       	call   40004be0 <close>
    40000cd4:	bf 18 67 00 40       	mov    $0x40006718,%edi
    40000cd9:	b8 00 00 00 00       	mov    $0x0,%eax
    40000cde:	e8 5d 17 00 00       	call   40002440 <printf>
    40000ce3:	48 8d 85 b0 f8 ff ff 	lea    -0x750(%rbp),%rax
    40000cea:	ba 10 00 00 00       	mov    $0x10,%edx
    40000cef:	48 89 c6             	mov    %rax,%rsi
    40000cf2:	bf 31 67 00 40       	mov    $0x40006731,%edi
    40000cf7:	e8 84 12 00 00       	call   40001f80 <getdents>
    40000cfc:	89 45 cc             	mov    %eax,-0x34(%rbp)
    40000cff:	8b 45 cc             	mov    -0x34(%rbp),%eax
    40000d02:	89 c6                	mov    %eax,%esi
    40000d04:	bf 33 67 00 40       	mov    $0x40006733,%edi
    40000d09:	b8 00 00 00 00       	mov    $0x0,%eax
    40000d0e:	e8 2d 17 00 00       	call   40002440 <printf>
    40000d13:	c7 45 fc 00 00 00 00 	movl   $0x0,-0x4(%rbp)
    40000d1a:	e9 a3 00 00 00       	jmp    40000dc2 <testAllSyscalls+0x38a>
    40000d1f:	48 8d 8d b0 f8 ff ff 	lea    -0x750(%rbp),%rcx
    40000d26:	8b 45 fc             	mov    -0x4(%rbp),%eax
    40000d29:	48 63 d0             	movslq %eax,%rdx
    40000d2c:	48 89 d0             	mov    %rdx,%rax
    40000d2f:	48 01 c0             	add    %rax,%rax
    40000d32:	48 01 d0             	add    %rdx,%rax
    40000d35:	48 8d 14 c5 00 00 00 	lea    0x0(,%rax,8),%rdx
    40000d3c:	00 
    40000d3d:	48 01 d0             	add    %rdx,%rax
    40000d40:	48 c1 e0 02          	shl    $0x2,%rax
    40000d44:	48 01 c8             	add    %rcx,%rax
    40000d47:	48 8d 70 05          	lea    0x5(%rax),%rsi
    40000d4b:	8b 45 fc             	mov    -0x4(%rbp),%eax
    40000d4e:	48 63 d0             	movslq %eax,%rdx
    40000d51:	48 89 d0             	mov    %rdx,%rax
    40000d54:	48 01 c0             	add    %rax,%rax
    40000d57:	48 01 d0             	add    %rdx,%rax
    40000d5a:	48 8d 14 c5 00 00 00 	lea    0x0(,%rax,8),%rdx
    40000d61:	00 
    40000d62:	48 01 d0             	add    %rdx,%rax
    40000d65:	48 c1 e0 02          	shl    $0x2,%rax
    40000d69:	48 01 e8             	add    %rbp,%rax
    40000d6c:	48 2d 4c 07 00 00    	sub    $0x74c,%rax
    40000d72:	0f b6 00             	movzbl (%rax),%eax
    40000d75:	0f b6 d0             	movzbl %al,%edx
    40000d78:	8b 45 fc             	mov    -0x4(%rbp),%eax
    40000d7b:	48 63 c8             	movslq %eax,%rcx
    40000d7e:	48 89 c8             	mov    %rcx,%rax
    40000d81:	48 01 c0             	add    %rax,%rax
    40000d84:	48 01 c8             	add    %rcx,%rax
    40000d87:	48 8d 0c c5 00 00 00 	lea    0x0(,%rax,8),%rcx
    40000d8e:	00 
    40000d8f:	48 01 c8             	add    %rcx,%rax
    40000d92:	48 c1 e0 02          	shl    $0x2,%rax
    40000d96:	48 01 e8             	add    %rbp,%rax
    40000d99:	48 2d 50 07 00 00    	sub    $0x750,%rax
    40000d9f:	8b 00                	mov    (%rax),%eax
    40000da1:	89 c7                	mov    %eax,%edi
    40000da3:	8b 45 fc             	mov    -0x4(%rbp),%eax
    40000da6:	49 89 f0             	mov    %rsi,%r8
    40000da9:	89 d1                	mov    %edx,%ecx
    40000dab:	89 fa                	mov    %edi,%edx
    40000dad:	89 c6                	mov    %eax,%esi
    40000daf:	bf 50 67 00 40       	mov    $0x40006750,%edi
    40000db4:	b8 00 00 00 00       	mov    $0x0,%eax
    40000db9:	e8 82 16 00 00       	call   40002440 <printf>
    40000dbe:	83 45 fc 01          	addl   $0x1,-0x4(%rbp)
    40000dc2:	8b 45 fc             	mov    -0x4(%rbp),%eax
    40000dc5:	3b 45 cc             	cmp    -0x34(%rbp),%eax
    40000dc8:	0f 8c 51 ff ff ff    	jl     40000d1f <testAllSyscalls+0x2e7>
    40000dce:	bf 6f 67 00 40       	mov    $0x4000676f,%edi
    40000dd3:	b8 00 00 00 00       	mov    $0x0,%eax
    40000dd8:	e8 63 16 00 00       	call   40002440 <printf>
    40000ddd:	ba 00 00 00 00       	mov    $0x0,%edx
    40000de2:	be 00 00 00 00       	mov    $0x0,%esi
    40000de7:	bf ab 65 00 40       	mov    $0x400065ab,%edi
    40000dec:	e8 af 10 00 00       	call   40001ea0 <open>
    40000df1:	89 45 f8             	mov    %eax,-0x8(%rbp)
    40000df4:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000df7:	89 c6                	mov    %eax,%esi
    40000df9:	bf 7f 67 00 40       	mov    $0x4000677f,%edi
    40000dfe:	b8 00 00 00 00       	mov    $0x0,%eax
    40000e03:	e8 38 16 00 00       	call   40002440 <printf>
    40000e08:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000e0b:	89 c7                	mov    %eax,%edi
    40000e0d:	e8 0e 3e 00 00       	call   40004c20 <dup>
    40000e12:	89 45 e4             	mov    %eax,-0x1c(%rbp)
    40000e15:	8b 45 e4             	mov    -0x1c(%rbp),%eax
    40000e18:	89 c6                	mov    %eax,%esi
    40000e1a:	bf 98 67 00 40       	mov    $0x40006798,%edi
    40000e1f:	b8 00 00 00 00       	mov    $0x0,%eax
    40000e24:	e8 17 16 00 00       	call   40002440 <printf>
    40000e29:	48 c7 85 a8 f8 ff ff 	movq   $0x0,-0x758(%rbp)
    40000e30:	00 00 00 00 
    40000e34:	48 c7 85 a0 f8 ff ff 	movq   $0x0,-0x760(%rbp)
    40000e3b:	00 00 00 00 
    40000e3f:	48 8d 8d a8 f8 ff ff 	lea    -0x758(%rbp),%rcx
    40000e46:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000e49:	ba 02 00 00 00       	mov    $0x2,%edx
    40000e4e:	48 89 ce             	mov    %rcx,%rsi
    40000e51:	89 c7                	mov    %eax,%edi
    40000e53:	e8 78 3e 00 00       	call   40004cd0 <read>
    40000e58:	48 8d 8d a0 f8 ff ff 	lea    -0x760(%rbp),%rcx
    40000e5f:	8b 45 e4             	mov    -0x1c(%rbp),%eax
    40000e62:	ba 02 00 00 00       	mov    $0x2,%edx
    40000e67:	48 89 ce             	mov    %rcx,%rsi
    40000e6a:	89 c7                	mov    %eax,%edi
    40000e6c:	e8 5f 3e 00 00       	call   40004cd0 <read>
    40000e71:	48 8d 95 a0 f8 ff ff 	lea    -0x760(%rbp),%rdx
    40000e78:	48 8d 85 a8 f8 ff ff 	lea    -0x758(%rbp),%rax
    40000e7f:	48 89 c6             	mov    %rax,%rsi
    40000e82:	bf ad 67 00 40       	mov    $0x400067ad,%edi
    40000e87:	b8 00 00 00 00       	mov    $0x0,%eax
    40000e8c:	e8 af 15 00 00       	call   40002440 <printf>
    40000e91:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000e94:	89 c7                	mov    %eax,%edi
    40000e96:	e8 45 3d 00 00       	call   40004be0 <close>
    40000e9b:	8b 45 e4             	mov    -0x1c(%rbp),%eax
    40000e9e:	89 c7                	mov    %eax,%edi
    40000ea0:	e8 3b 3d 00 00       	call   40004be0 <close>
    40000ea5:	bf ca 67 00 40       	mov    $0x400067ca,%edi
    40000eaa:	b8 00 00 00 00       	mov    $0x0,%eax
    40000eaf:	e8 8c 15 00 00       	call   40002440 <printf>
    40000eb4:	ba 00 00 00 00       	mov    $0x0,%edx
    40000eb9:	be 00 00 00 00       	mov    $0x0,%esi
    40000ebe:	bf ab 65 00 40       	mov    $0x400065ab,%edi
    40000ec3:	e8 d8 0f 00 00       	call   40001ea0 <open>
    40000ec8:	89 45 c8             	mov    %eax,-0x38(%rbp)
    40000ecb:	c7 45 c4 0a 00 00 00 	movl   $0xa,-0x3c(%rbp)
    40000ed2:	8b 55 c4             	mov    -0x3c(%rbp),%edx
    40000ed5:	8b 45 c8             	mov    -0x38(%rbp),%eax
    40000ed8:	89 d6                	mov    %edx,%esi
    40000eda:	89 c7                	mov    %eax,%edi
    40000edc:	e8 5f 3d 00 00       	call   40004c40 <dup2>
    40000ee1:	89 45 c0             	mov    %eax,-0x40(%rbp)
    40000ee4:	8b 55 c4             	mov    -0x3c(%rbp),%edx
    40000ee7:	8b 45 c0             	mov    -0x40(%rbp),%eax
    40000eea:	89 c6                	mov    %eax,%esi
    40000eec:	bf db 67 00 40       	mov    $0x400067db,%edi
    40000ef1:	b8 00 00 00 00       	mov    $0x0,%eax
    40000ef6:	e8 45 15 00 00       	call   40002440 <printf>
    40000efb:	8b 45 c8             	mov    -0x38(%rbp),%eax
    40000efe:	89 c7                	mov    %eax,%edi
    40000f00:	e8 db 3c 00 00       	call   40004be0 <close>
    40000f05:	8b 45 c4             	mov    -0x3c(%rbp),%eax
    40000f08:	89 c7                	mov    %eax,%edi
    40000f0a:	e8 d1 3c 00 00       	call   40004be0 <close>
    40000f0f:	bf f8 67 00 40       	mov    $0x400067f8,%edi
    40000f14:	b8 00 00 00 00       	mov    $0x0,%eax
    40000f19:	e8 22 15 00 00       	call   40002440 <printf>
    40000f1e:	ba 00 00 00 00       	mov    $0x0,%edx
    40000f23:	be 00 00 00 00       	mov    $0x0,%esi
    40000f28:	bf ab 65 00 40       	mov    $0x400065ab,%edi
    40000f2d:	e8 6e 0f 00 00       	call   40001ea0 <open>
    40000f32:	89 45 f8             	mov    %eax,-0x8(%rbp)
    40000f35:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000f38:	89 c6                	mov    %eax,%esi
    40000f3a:	bf 0a 68 00 40       	mov    $0x4000680a,%edi
    40000f3f:	b8 00 00 00 00       	mov    $0x0,%eax
    40000f44:	e8 f7 14 00 00       	call   40002440 <printf>
    40000f49:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000f4c:	ba 00 00 00 00       	mov    $0x0,%edx
    40000f51:	be 03 00 00 00       	mov    $0x3,%esi
    40000f56:	89 c7                	mov    %eax,%edi
    40000f58:	e8 b3 0d 00 00       	call   40001d10 <fcntl>
    40000f5d:	89 45 bc             	mov    %eax,-0x44(%rbp)
    40000f60:	8b 45 bc             	mov    -0x44(%rbp),%eax
    40000f63:	89 c6                	mov    %eax,%esi
    40000f65:	bf 25 68 00 40       	mov    $0x40006825,%edi
    40000f6a:	b8 00 00 00 00       	mov    $0x0,%eax
    40000f6f:	e8 cc 14 00 00       	call   40002440 <printf>
    40000f74:	8b 45 bc             	mov    -0x44(%rbp),%eax
    40000f77:	80 cc 04             	or     $0x4,%ah
    40000f7a:	48 63 d0             	movslq %eax,%rdx
    40000f7d:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000f80:	be 04 00 00 00       	mov    $0x4,%esi
    40000f85:	89 c7                	mov    %eax,%edi
    40000f87:	e8 84 0d 00 00       	call   40001d10 <fcntl>
    40000f8c:	89 45 c0             	mov    %eax,-0x40(%rbp)
    40000f8f:	8b 45 c0             	mov    -0x40(%rbp),%eax
    40000f92:	89 c6                	mov    %eax,%esi
    40000f94:	bf 3c 68 00 40       	mov    $0x4000683c,%edi
    40000f99:	b8 00 00 00 00       	mov    $0x0,%eax
    40000f9e:	e8 9d 14 00 00       	call   40002440 <printf>
    40000fa3:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000fa6:	ba 00 00 00 00       	mov    $0x0,%edx
    40000fab:	be 03 00 00 00       	mov    $0x3,%esi
    40000fb0:	89 c7                	mov    %eax,%edi
    40000fb2:	e8 59 0d 00 00       	call   40001d10 <fcntl>
    40000fb7:	89 45 b8             	mov    %eax,-0x48(%rbp)
    40000fba:	8b 45 b8             	mov    -0x48(%rbp),%eax
    40000fbd:	89 c6                	mov    %eax,%esi
    40000fbf:	bf 58 68 00 40       	mov    $0x40006858,%edi
    40000fc4:	b8 00 00 00 00       	mov    $0x0,%eax
    40000fc9:	e8 72 14 00 00       	call   40002440 <printf>
    40000fce:	8b 45 f8             	mov    -0x8(%rbp),%eax
    40000fd1:	89 c7                	mov    %eax,%edi
    40000fd3:	e8 08 3c 00 00       	call   40004be0 <close>
    40000fd8:	bf 77 68 00 40       	mov    $0x40006877,%edi
    40000fdd:	b8 00 00 00 00       	mov    $0x0,%eax
    40000fe2:	e8 59 14 00 00       	call   40002440 <printf>
    40000fe7:	48 8d 85 98 f8 ff ff 	lea    -0x768(%rbp),%rax
    40000fee:	48 89 c7             	mov    %rax,%rdi
    40000ff1:	e8 ba 3c 00 00       	call   40004cb0 <pipe>
    40000ff6:	85 c0                	test   %eax,%eax
    40000ff8:	0f 85 e0 00 00 00    	jne    400010de <testAllSyscalls+0x6a6>
    40000ffe:	8b 95 9c f8 ff ff    	mov    -0x764(%rbp),%edx
    40001004:	8b 85 98 f8 ff ff    	mov    -0x768(%rbp),%eax
    4000100a:	89 c6                	mov    %eax,%esi
    4000100c:	bf 88 68 00 40       	mov    $0x40006888,%edi
    40001011:	b8 00 00 00 00       	mov    $0x0,%eax
    40001016:	e8 25 14 00 00       	call   40002440 <printf>
    4000101b:	48 c7 45 b0 a1 68 00 	movq   $0x400068a1,-0x50(%rbp)
    40001022:	40 
    40001023:	8b 85 9c f8 ff ff    	mov    -0x764(%rbp),%eax
    40001029:	48 8b 4d b0          	mov    -0x50(%rbp),%rcx
    4000102d:	ba 0f 00 00 00       	mov    $0xf,%edx
    40001032:	48 89 ce             	mov    %rcx,%rsi
    40001035:	89 c7                	mov    %eax,%edi
    40001037:	e8 c4 3c 00 00       	call   40004d00 <write>
    4000103c:	48 89 45 a8          	mov    %rax,-0x58(%rbp)
    40001040:	48 8b 45 a8          	mov    -0x58(%rbp),%rax
    40001044:	89 c6                	mov    %eax,%esi
    40001046:	bf b2 68 00 40       	mov    $0x400068b2,%edi
    4000104b:	b8 00 00 00 00       	mov    $0x0,%eax
    40001050:	e8 eb 13 00 00       	call   40002440 <printf>
    40001055:	48 c7 85 c0 f7 ff ff 	movq   $0x0,-0x840(%rbp)
    4000105c:	00 00 00 00 
    40001060:	48 c7 85 c8 f7 ff ff 	movq   $0x0,-0x838(%rbp)
    40001067:	00 00 00 00 
    4000106b:	48 c7 85 d0 f7 ff ff 	movq   $0x0,-0x830(%rbp)
    40001072:	00 00 00 00 
    40001076:	48 c7 85 d8 f7 ff ff 	movq   $0x0,-0x828(%rbp)
    4000107d:	00 00 00 00 
    40001081:	8b 85 98 f8 ff ff    	mov    -0x768(%rbp),%eax
    40001087:	48 8d 8d c0 f7 ff ff 	lea    -0x840(%rbp),%rcx
    4000108e:	ba 1f 00 00 00       	mov    $0x1f,%edx
    40001093:	48 89 ce             	mov    %rcx,%rsi
    40001096:	89 c7                	mov    %eax,%edi
    40001098:	e8 33 3c 00 00       	call   40004cd0 <read>
    4000109d:	48 89 45 a0          	mov    %rax,-0x60(%rbp)
    400010a1:	48 8b 45 a0          	mov    -0x60(%rbp),%rax
    400010a5:	89 c1                	mov    %eax,%ecx
    400010a7:	48 8d 85 c0 f7 ff ff 	lea    -0x840(%rbp),%rax
    400010ae:	48 89 c2             	mov    %rax,%rdx
    400010b1:	89 ce                	mov    %ecx,%esi
    400010b3:	bf d0 68 00 40       	mov    $0x400068d0,%edi
    400010b8:	b8 00 00 00 00       	mov    $0x0,%eax
    400010bd:	e8 7e 13 00 00       	call   40002440 <printf>
    400010c2:	8b 85 98 f8 ff ff    	mov    -0x768(%rbp),%eax
    400010c8:	89 c7                	mov    %eax,%edi
    400010ca:	e8 11 3b 00 00       	call   40004be0 <close>
    400010cf:	8b 85 9c f8 ff ff    	mov    -0x764(%rbp),%eax
    400010d5:	89 c7                	mov    %eax,%edi
    400010d7:	e8 04 3b 00 00       	call   40004be0 <close>
    400010dc:	eb 0f                	jmp    400010ed <testAllSyscalls+0x6b5>
    400010de:	bf f1 68 00 40       	mov    $0x400068f1,%edi
    400010e3:	b8 00 00 00 00       	mov    $0x0,%eax
    400010e8:	e8 53 13 00 00       	call   40002440 <printf>
    400010ed:	bf fe 68 00 40       	mov    $0x400068fe,%edi
    400010f2:	e8 92 08 00 00       	call   40001989 <klog>
    400010f7:	48 8d 85 80 f8 ff ff 	lea    -0x780(%rbp),%rax
    400010fe:	48 89 c6             	mov    %rax,%rsi
    40001101:	bf 01 00 00 00       	mov    $0x1,%edi
    40001106:	e8 f5 39 00 00       	call   40004b00 <__clock_gettime>
    4000110b:	85 c0                	test   %eax,%eax
    4000110d:	75 23                	jne    40001132 <testAllSyscalls+0x6fa>
    4000110f:	48 8b 85 88 f8 ff ff 	mov    -0x778(%rbp),%rax
    40001116:	89 c2                	mov    %eax,%edx
    40001118:	48 8b 85 80 f8 ff ff 	mov    -0x780(%rbp),%rax
    4000111f:	89 c6                	mov    %eax,%esi
    40001121:	bf 18 69 00 40       	mov    $0x40006918,%edi
    40001126:	b8 00 00 00 00       	mov    $0x0,%eax
    4000112b:	e8 10 13 00 00       	call   40002440 <printf>
    40001130:	eb 19                	jmp    4000114b <testAllSyscalls+0x713>
    40001132:	bf 34 69 00 40       	mov    $0x40006934,%edi
    40001137:	b8 00 00 00 00       	mov    $0x0,%eax
    4000113c:	e8 ff 12 00 00       	call   40002440 <printf>
    40001141:	bf 34 69 00 40       	mov    $0x40006934,%edi
    40001146:	e8 3e 08 00 00       	call   40001989 <klog>
    4000114b:	48 8d 85 80 f8 ff ff 	lea    -0x780(%rbp),%rax
    40001152:	48 89 c6             	mov    %rax,%rsi
    40001155:	bf 00 00 00 00       	mov    $0x0,%edi
    4000115a:	e8 a1 39 00 00       	call   40004b00 <__clock_gettime>
    4000115f:	85 c0                	test   %eax,%eax
    40001161:	75 21                	jne    40001184 <testAllSyscalls+0x74c>
    40001163:	48 8b 85 88 f8 ff ff 	mov    -0x778(%rbp),%rax
    4000116a:	89 c2                	mov    %eax,%edx
    4000116c:	48 8b 85 80 f8 ff ff 	mov    -0x780(%rbp),%rax
    40001173:	89 c6                	mov    %eax,%esi
    40001175:	bf 4a 69 00 40       	mov    $0x4000694a,%edi
    4000117a:	b8 00 00 00 00       	mov    $0x0,%eax
    4000117f:	e8 bc 12 00 00       	call   40002440 <printf>
    40001184:	bf 65 69 00 40       	mov    $0x40006965,%edi
    40001189:	b8 00 00 00 00       	mov    $0x0,%eax
    4000118e:	e8 ad 12 00 00       	call   40002440 <printf>
    40001193:	48 c7 85 70 f8 ff ff 	movq   $0x0,-0x790(%rbp)
    4000119a:	00 00 00 00 
    4000119e:	48 c7 85 78 f8 ff ff 	movq   $0x1dcd6500,-0x788(%rbp)
    400011a5:	00 65 cd 1d 
    400011a9:	48 8d 85 60 f8 ff ff 	lea    -0x7a0(%rbp),%rax
    400011b0:	48 89 c6             	mov    %rax,%rsi
    400011b3:	bf 01 00 00 00       	mov    $0x1,%edi
    400011b8:	e8 43 39 00 00       	call   40004b00 <__clock_gettime>
    400011bd:	48 8d 85 70 f8 ff ff 	lea    -0x790(%rbp),%rax
    400011c4:	be 00 00 00 00       	mov    $0x0,%esi
    400011c9:	48 89 c7             	mov    %rax,%rdi
    400011cc:	e8 cf 39 00 00       	call   40004ba0 <nanosleep>
    400011d1:	48 8d 85 50 f8 ff ff 	lea    -0x7b0(%rbp),%rax
    400011d8:	48 89 c6             	mov    %rax,%rsi
    400011db:	bf 01 00 00 00       	mov    $0x1,%edi
    400011e0:	e8 1b 39 00 00       	call   40004b00 <__clock_gettime>
    400011e5:	48 8b 85 50 f8 ff ff 	mov    -0x7b0(%rbp),%rax
    400011ec:	89 c2                	mov    %eax,%edx
    400011ee:	48 8b 85 60 f8 ff ff 	mov    -0x7a0(%rbp),%rax
    400011f5:	89 c1                	mov    %eax,%ecx
    400011f7:	89 d0                	mov    %edx,%eax
    400011f9:	29 c8                	sub    %ecx,%eax
    400011fb:	89 c6                	mov    %eax,%esi
    400011fd:	bf 7b 69 00 40       	mov    $0x4000697b,%edi
    40001202:	b8 00 00 00 00       	mov    $0x0,%eax
    40001207:	e8 34 12 00 00       	call   40002440 <printf>
    4000120c:	48 8b 85 58 f8 ff ff 	mov    -0x7a8(%rbp),%rax
    40001213:	89 c2                	mov    %eax,%edx
    40001215:	48 8b 85 68 f8 ff ff 	mov    -0x798(%rbp),%rax
    4000121c:	89 c1                	mov    %eax,%ecx
    4000121e:	89 d0                	mov    %edx,%eax
    40001220:	29 c8                	sub    %ecx,%eax
    40001222:	89 c7                	mov    %eax,%edi
    40001224:	e8 8f f0 ff ff       	call   400002b8 <print_ns>
    40001229:	bf 86 69 00 40       	mov    $0x40006986,%edi
    4000122e:	b8 00 00 00 00       	mov    $0x0,%eax
    40001233:	e8 08 12 00 00       	call   40002440 <printf>
    40001238:	48 83 ec 08          	sub    $0x8,%rsp
    4000123c:	6a 00                	push   $0x0
    4000123e:	41 b9 00 00 00 00    	mov    $0x0,%r9d
    40001244:	49 b8 77 77 66 66 55 	movabs $0x4444555566667777,%r8
    4000124b:	55 44 44 
    4000124e:	b9 33 00 00 00       	mov    $0x33,%ecx
    40001253:	ba 22 00 00 00       	mov    $0x22,%edx
    40001258:	be 11 00 00 00       	mov    $0x11,%esi
    4000125d:	bf e7 03 00 00       	mov    $0x3e7,%edi
    40001262:	e8 30 06 00 00       	call   40001897 <syscall6>
    40001267:	48 83 c4 10          	add    $0x10,%rsp
    4000126b:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    4000126f:	48 8b 45 98          	mov    -0x68(%rbp),%rax
    40001273:	48 89 c6             	mov    %rax,%rsi
    40001276:	bf 95 69 00 40       	mov    $0x40006995,%edi
    4000127b:	b8 00 00 00 00       	mov    $0x0,%eax
    40001280:	e8 bb 11 00 00       	call   40002440 <printf>
    40001285:	bf b3 69 00 40       	mov    $0x400069b3,%edi
    4000128a:	b8 00 00 00 00       	mov    $0x0,%eax
    4000128f:	e8 ac 11 00 00       	call   40002440 <printf>
    40001294:	48 8d 85 48 f8 ff ff 	lea    -0x7b8(%rbp),%rax
    4000129b:	48 89 c1             	mov    %rax,%rcx
    4000129e:	ba 00 00 00 00       	mov    $0x0,%edx
    400012a3:	be 01 00 00 00       	mov    $0x1,%esi
    400012a8:	bf 01 00 00 00       	mov    $0x1,%edi
    400012ad:	e8 3e 10 00 00       	call   400022f0 <socketpair>
    400012b2:	85 c0                	test   %eax,%eax
    400012b4:	0f 85 b4 00 00 00    	jne    4000136e <testAllSyscalls+0x936>
    400012ba:	8b 95 4c f8 ff ff    	mov    -0x7b4(%rbp),%edx
    400012c0:	8b 85 48 f8 ff ff    	mov    -0x7b8(%rbp),%eax
    400012c6:	89 c6                	mov    %eax,%esi
    400012c8:	bf ca 69 00 40       	mov    $0x400069ca,%edi
    400012cd:	b8 00 00 00 00       	mov    $0x0,%eax
    400012d2:	e8 69 11 00 00       	call   40002440 <printf>
    400012d7:	8b 85 48 f8 ff ff    	mov    -0x7b8(%rbp),%eax
    400012dd:	ba 09 00 00 00       	mov    $0x9,%edx
    400012e2:	be e6 69 00 40       	mov    $0x400069e6,%esi
    400012e7:	89 c7                	mov    %eax,%edi
    400012e9:	e8 12 3a 00 00       	call   40004d00 <write>
    400012ee:	48 c7 85 c0 f7 ff ff 	movq   $0x0,-0x840(%rbp)
    400012f5:	00 00 00 00 
    400012f9:	48 c7 85 c8 f7 ff ff 	movq   $0x0,-0x838(%rbp)
    40001300:	00 00 00 00 
    40001304:	48 c7 85 d0 f7 ff ff 	movq   $0x0,-0x830(%rbp)
    4000130b:	00 00 00 00 
    4000130f:	48 c7 85 d8 f7 ff ff 	movq   $0x0,-0x828(%rbp)
    40001316:	00 00 00 00 
    4000131a:	8b 85 4c f8 ff ff    	mov    -0x7b4(%rbp),%eax
    40001320:	48 8d 8d c0 f7 ff ff 	lea    -0x840(%rbp),%rcx
    40001327:	ba 1f 00 00 00       	mov    $0x1f,%edx
    4000132c:	48 89 ce             	mov    %rcx,%rsi
    4000132f:	89 c7                	mov    %eax,%edi
    40001331:	e8 9a 39 00 00       	call   40004cd0 <read>
    40001336:	89 45 94             	mov    %eax,-0x6c(%rbp)
    40001339:	48 8d 95 c0 f7 ff ff 	lea    -0x840(%rbp),%rdx
    40001340:	8b 45 94             	mov    -0x6c(%rbp),%eax
    40001343:	89 c6                	mov    %eax,%esi
    40001345:	bf f0 69 00 40       	mov    $0x400069f0,%edi
    4000134a:	b8 00 00 00 00       	mov    $0x0,%eax
    4000134f:	e8 ec 10 00 00       	call   40002440 <printf>
    40001354:	8b 85 48 f8 ff ff    	mov    -0x7b8(%rbp),%eax
    4000135a:	89 c7                	mov    %eax,%edi
    4000135c:	e8 7f 38 00 00       	call   40004be0 <close>
    40001361:	8b 85 4c f8 ff ff    	mov    -0x7b4(%rbp),%eax
    40001367:	89 c7                	mov    %eax,%edi
    40001369:	e8 72 38 00 00       	call   40004be0 <close>
    4000136e:	bf 18 6a 00 40       	mov    $0x40006a18,%edi
    40001373:	b8 00 00 00 00       	mov    $0x0,%eax
    40001378:	e8 c3 10 00 00       	call   40002440 <printf>
    4000137d:	e8 45 f6 ff ff       	call   400009c7 <test_raw_socketpair>
    40001382:	bf 38 6a 00 40       	mov    $0x40006a38,%edi
    40001387:	b8 00 00 00 00       	mov    $0x0,%eax
    4000138c:	e8 af 10 00 00       	call   40002440 <printf>
    40001391:	e8 a8 ef ff ff       	call   4000033e <test_socketpair_poll>
    40001396:	bf 60 6a 00 40       	mov    $0x40006a60,%edi
    4000139b:	b8 00 00 00 00       	mov    $0x0,%eax
    400013a0:	e8 9b 10 00 00       	call   40002440 <printf>
    400013a5:	e8 4f f1 ff ff       	call   400004f9 <test_socketpair_nonblock>
    400013aa:	bf 88 6a 00 40       	mov    $0x40006a88,%edi
    400013af:	b8 00 00 00 00       	mov    $0x0,%eax
    400013b4:	e8 87 10 00 00       	call   40002440 <printf>
    400013b9:	e8 4c f2 ff ff       	call   4000060a <test_syscalls_presence>
    400013be:	bf a8 6a 00 40       	mov    $0x40006aa8,%edi
    400013c3:	b8 00 00 00 00       	mov    $0x0,%eax
    400013c8:	e8 73 10 00 00       	call   40002440 <printf>
    400013cd:	e8 1e f3 ff ff       	call   400006f0 <test_memfd>
    400013d2:	bf bc 6a 00 40       	mov    $0x40006abc,%edi
    400013d7:	b8 00 00 00 00       	mov    $0x0,%eax
    400013dc:	e8 5f 10 00 00       	call   40002440 <printf>
    400013e1:	e8 cd f3 ff ff       	call   400007b3 <test_ftruncate_memfd>
    400013e6:	bf da 6a 00 40       	mov    $0x40006ada,%edi
    400013eb:	b8 00 00 00 00       	mov    $0x0,%eax
    400013f0:	e8 4b 10 00 00       	call   40002440 <printf>
    400013f5:	e8 63 f4 ff ff       	call   4000085d <test_memfd_mmap>
    400013fa:	90                   	nop
    400013fb:	c9                   	leave  
    400013fc:	c3                   	ret    

00000000400013fd <my_raw_syscall6>:
    400013fd:	55                   	push   %rbp
    400013fe:	48 89 e5             	mov    %rsp,%rbp
    40001401:	48 89 7d e8          	mov    %rdi,-0x18(%rbp)
    40001405:	48 89 75 e0          	mov    %rsi,-0x20(%rbp)
    40001409:	48 89 55 d8          	mov    %rdx,-0x28(%rbp)
    4000140d:	48 89 4d d0          	mov    %rcx,-0x30(%rbp)
    40001411:	4c 89 45 c8          	mov    %r8,-0x38(%rbp)
    40001415:	4c 89 4d c0          	mov    %r9,-0x40(%rbp)
    40001419:	4c 8b 55 c8          	mov    -0x38(%rbp),%r10
    4000141d:	4c 8b 45 c0          	mov    -0x40(%rbp),%r8
    40001421:	4c 8b 4d 10          	mov    0x10(%rbp),%r9
    40001425:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    40001429:	48 8b 7d e0          	mov    -0x20(%rbp),%rdi
    4000142d:	48 8b 75 d8          	mov    -0x28(%rbp),%rsi
    40001431:	48 8b 55 d0          	mov    -0x30(%rbp),%rdx
    40001435:	0f 05                	syscall 
    40001437:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    4000143b:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    4000143f:	5d                   	pop    %rbp
    40001440:	c3                   	ret    

0000000040001441 <main>:
    40001441:	55                   	push   %rbp
    40001442:	48 89 e5             	mov    %rsp,%rbp
    40001445:	48 83 ec 30          	sub    $0x30,%rsp
    40001449:	bf f8 6a 00 40       	mov    $0x40006af8,%edi
    4000144e:	e8 36 05 00 00       	call   40001989 <klog>
    40001453:	bf 28 6b 00 40       	mov    $0x40006b28,%edi
    40001458:	b8 00 00 00 00       	mov    $0x0,%eax
    4000145d:	e8 de 0f 00 00       	call   40002440 <printf>
    40001462:	48 83 ec 08          	sub    $0x8,%rsp
    40001466:	6a 66                	push   $0x66
    40001468:	41 b9 55 00 00 00    	mov    $0x55,%r9d
    4000146e:	41 b8 44 00 00 00    	mov    $0x44,%r8d
    40001474:	b9 33 00 00 00       	mov    $0x33,%ecx
    40001479:	ba 22 00 00 00       	mov    $0x22,%edx
    4000147e:	be 11 00 00 00       	mov    $0x11,%esi
    40001483:	bf e7 03 00 00       	mov    $0x3e7,%edi
    40001488:	e8 70 ff ff ff       	call   400013fd <my_raw_syscall6>
    4000148d:	48 83 c4 10          	add    $0x10,%rsp
    40001491:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    40001495:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    40001499:	48 89 c6             	mov    %rax,%rsi
    4000149c:	bf 4b 6b 00 40       	mov    $0x40006b4b,%edi
    400014a1:	b8 00 00 00 00       	mov    $0x0,%eax
    400014a6:	e8 95 0f 00 00       	call   40002440 <printf>
    400014ab:	48 b8 48 65 6c 6c 6f 	movabs $0x7266206f6c6c6548,%rax
    400014b2:	20 66 72 
    400014b5:	48 ba 6f 6d 20 53 59 	movabs $0x4143535953206d6f,%rdx
    400014bc:	53 43 41 
    400014bf:	48 89 45 d0          	mov    %rax,-0x30(%rbp)
    400014c3:	48 89 55 d8          	mov    %rdx,-0x28(%rbp)
    400014c7:	48 b8 4c 4c 20 75 73 	movabs $0x6c72657375204c4c,%rax
    400014ce:	65 72 6c 
    400014d1:	48 89 45 e0          	mov    %rax,-0x20(%rbp)
    400014d5:	c7 45 e8 61 6e 64 21 	movl   $0x21646e61,-0x18(%rbp)
    400014dc:	66 c7 45 ec 0a 00    	movw   $0xa,-0x14(%rbp)
    400014e2:	48 8d 45 d0          	lea    -0x30(%rbp),%rax
    400014e6:	48 83 ec 08          	sub    $0x8,%rsp
    400014ea:	6a 00                	push   $0x0
    400014ec:	41 b9 00 00 00 00    	mov    $0x0,%r9d
    400014f2:	41 b8 00 00 00 00    	mov    $0x0,%r8d
    400014f8:	b9 1d 00 00 00       	mov    $0x1d,%ecx
    400014fd:	48 89 c2             	mov    %rax,%rdx
    40001500:	be 01 00 00 00       	mov    $0x1,%esi
    40001505:	bf 01 00 00 00       	mov    $0x1,%edi
    4000150a:	e8 ee fe ff ff       	call   400013fd <my_raw_syscall6>
    4000150f:	48 83 c4 10          	add    $0x10,%rsp
    40001513:	bf 68 6b 00 40       	mov    $0x40006b68,%edi
    40001518:	b8 00 00 00 00       	mov    $0x0,%eax
    4000151d:	e8 1e 0f 00 00       	call   40002440 <printf>
    40001522:	48 83 ec 08          	sub    $0x8,%rsp
    40001526:	6a 00                	push   $0x0
    40001528:	41 b9 00 00 00 00    	mov    $0x0,%r9d
    4000152e:	41 b8 00 00 00 00    	mov    $0x0,%r8d
    40001534:	b9 00 00 00 00       	mov    $0x0,%ecx
    40001539:	ba 00 00 00 00       	mov    $0x0,%edx
    4000153e:	be 00 00 00 00       	mov    $0x0,%esi
    40001543:	bf 3c 00 00 00       	mov    $0x3c,%edi
    40001548:	e8 b0 fe ff ff       	call   400013fd <my_raw_syscall6>
    4000154d:	48 83 c4 10          	add    $0x10,%rsp
    40001551:	bf 8c 6b 00 40       	mov    $0x40006b8c,%edi
    40001556:	b8 00 00 00 00       	mov    $0x0,%eax
    4000155b:	e8 e0 0e 00 00       	call   40002440 <printf>
    40001560:	b8 00 00 00 00       	mov    $0x0,%eax
    40001565:	c9                   	leave  
    40001566:	c3                   	ret    

0000000040001567 <__syscall>:
    40001567:	55                   	push   %rbp
    40001568:	48 89 e5             	mov    %rsp,%rbp
    4000156b:	48 83 ec 30          	sub    $0x30,%rsp
    4000156f:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
    40001573:	48 89 75 f0          	mov    %rsi,-0x10(%rbp)
    40001577:	48 89 55 e8          	mov    %rdx,-0x18(%rbp)
    4000157b:	48 89 4d e0          	mov    %rcx,-0x20(%rbp)
    4000157f:	4c 89 45 d8          	mov    %r8,-0x28(%rbp)
    40001583:	4c 89 4d d0          	mov    %r9,-0x30(%rbp)
    40001587:	4c 8b 45 d0          	mov    -0x30(%rbp),%r8
    4000158b:	48 8b 7d d8          	mov    -0x28(%rbp),%rdi
    4000158f:	48 8b 4d e0          	mov    -0x20(%rbp),%rcx
    40001593:	48 8b 55 e8          	mov    -0x18(%rbp),%rdx
    40001597:	48 8b 75 f0          	mov    -0x10(%rbp),%rsi
    4000159b:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    4000159f:	48 83 ec 08          	sub    $0x8,%rsp
    400015a3:	ff 75 10             	push   0x10(%rbp)
    400015a6:	4d 89 c1             	mov    %r8,%r9
    400015a9:	49 89 f8             	mov    %rdi,%r8
    400015ac:	48 89 c7             	mov    %rax,%rdi
    400015af:	e8 e3 02 00 00       	call   40001897 <syscall6>
    400015b4:	48 83 c4 10          	add    $0x10,%rsp
    400015b8:	c9                   	leave  
    400015b9:	c3                   	ret    

00000000400015ba <__syscall_cp>:
    400015ba:	55                   	push   %rbp
    400015bb:	48 89 e5             	mov    %rsp,%rbp
    400015be:	48 83 ec 30          	sub    $0x30,%rsp
    400015c2:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
    400015c6:	48 89 75 f0          	mov    %rsi,-0x10(%rbp)
    400015ca:	48 89 55 e8          	mov    %rdx,-0x18(%rbp)
    400015ce:	48 89 4d e0          	mov    %rcx,-0x20(%rbp)
    400015d2:	4c 89 45 d8          	mov    %r8,-0x28(%rbp)
    400015d6:	4c 89 4d d0          	mov    %r9,-0x30(%rbp)
    400015da:	4c 8b 45 d0          	mov    -0x30(%rbp),%r8
    400015de:	48 8b 7d d8          	mov    -0x28(%rbp),%rdi
    400015e2:	48 8b 4d e0          	mov    -0x20(%rbp),%rcx
    400015e6:	48 8b 55 e8          	mov    -0x18(%rbp),%rdx
    400015ea:	48 8b 75 f0          	mov    -0x10(%rbp),%rsi
    400015ee:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    400015f2:	48 83 ec 08          	sub    $0x8,%rsp
    400015f6:	ff 75 10             	push   0x10(%rbp)
    400015f9:	4d 89 c1             	mov    %r8,%r9
    400015fc:	49 89 f8             	mov    %rdi,%r8
    400015ff:	48 89 c7             	mov    %rax,%rdi
    40001602:	e8 90 02 00 00       	call   40001897 <syscall6>
    40001607:	48 83 c4 10          	add    $0x10,%rsp
    4000160b:	c9                   	leave  
    4000160c:	c3                   	ret    

000000004000160d <syscall>:
    4000160d:	55                   	push   %rbp
    4000160e:	48 89 e5             	mov    %rsp,%rbp
    40001611:	48 81 ec 10 01 00 00 	sub    $0x110,%rsp
    40001618:	48 89 bd f8 fe ff ff 	mov    %rdi,-0x108(%rbp)
    4000161f:	48 89 b5 58 ff ff ff 	mov    %rsi,-0xa8(%rbp)
    40001626:	48 89 95 60 ff ff ff 	mov    %rdx,-0xa0(%rbp)
    4000162d:	48 89 8d 68 ff ff ff 	mov    %rcx,-0x98(%rbp)
    40001634:	4c 89 85 70 ff ff ff 	mov    %r8,-0x90(%rbp)
    4000163b:	4c 89 8d 78 ff ff ff 	mov    %r9,-0x88(%rbp)
    40001642:	84 c0                	test   %al,%al
    40001644:	74 20                	je     40001666 <syscall+0x59>
    40001646:	0f 29 45 80          	movaps %xmm0,-0x80(%rbp)
    4000164a:	0f 29 4d 90          	movaps %xmm1,-0x70(%rbp)
    4000164e:	0f 29 55 a0          	movaps %xmm2,-0x60(%rbp)
    40001652:	0f 29 5d b0          	movaps %xmm3,-0x50(%rbp)
    40001656:	0f 29 65 c0          	movaps %xmm4,-0x40(%rbp)
    4000165a:	0f 29 6d d0          	movaps %xmm5,-0x30(%rbp)
    4000165e:	0f 29 75 e0          	movaps %xmm6,-0x20(%rbp)
    40001662:	0f 29 7d f0          	movaps %xmm7,-0x10(%rbp)
    40001666:	c7 85 08 ff ff ff 08 	movl   $0x8,-0xf8(%rbp)
    4000166d:	00 00 00 
    40001670:	c7 85 0c ff ff ff 30 	movl   $0x30,-0xf4(%rbp)
    40001677:	00 00 00 
    4000167a:	48 8d 45 10          	lea    0x10(%rbp),%rax
    4000167e:	48 89 85 10 ff ff ff 	mov    %rax,-0xf0(%rbp)
    40001685:	48 8d 85 50 ff ff ff 	lea    -0xb0(%rbp),%rax
    4000168c:	48 89 85 18 ff ff ff 	mov    %rax,-0xe8(%rbp)
    40001693:	8b 85 08 ff ff ff    	mov    -0xf8(%rbp),%eax
    40001699:	83 f8 2f             	cmp    $0x2f,%eax
    4000169c:	77 23                	ja     400016c1 <syscall+0xb4>
    4000169e:	48 8b 85 18 ff ff ff 	mov    -0xe8(%rbp),%rax
    400016a5:	8b 95 08 ff ff ff    	mov    -0xf8(%rbp),%edx
    400016ab:	89 d2                	mov    %edx,%edx
    400016ad:	48 01 d0             	add    %rdx,%rax
    400016b0:	8b 95 08 ff ff ff    	mov    -0xf8(%rbp),%edx
    400016b6:	83 c2 08             	add    $0x8,%edx
    400016b9:	89 95 08 ff ff ff    	mov    %edx,-0xf8(%rbp)
    400016bf:	eb 12                	jmp    400016d3 <syscall+0xc6>
    400016c1:	48 8b 85 10 ff ff ff 	mov    -0xf0(%rbp),%rax
    400016c8:	48 8d 50 08          	lea    0x8(%rax),%rdx
    400016cc:	48 89 95 10 ff ff ff 	mov    %rdx,-0xf0(%rbp)
    400016d3:	48 8b 00             	mov    (%rax),%rax
    400016d6:	48 89 85 48 ff ff ff 	mov    %rax,-0xb8(%rbp)
    400016dd:	8b 85 08 ff ff ff    	mov    -0xf8(%rbp),%eax
    400016e3:	83 f8 2f             	cmp    $0x2f,%eax
    400016e6:	77 23                	ja     4000170b <syscall+0xfe>
    400016e8:	48 8b 85 18 ff ff ff 	mov    -0xe8(%rbp),%rax
    400016ef:	8b 95 08 ff ff ff    	mov    -0xf8(%rbp),%edx
    400016f5:	89 d2                	mov    %edx,%edx
    400016f7:	48 01 d0             	add    %rdx,%rax
    400016fa:	8b 95 08 ff ff ff    	mov    -0xf8(%rbp),%edx
    40001700:	83 c2 08             	add    $0x8,%edx
    40001703:	89 95 08 ff ff ff    	mov    %edx,-0xf8(%rbp)
    40001709:	eb 12                	jmp    4000171d <syscall+0x110>
    4000170b:	48 8b 85 10 ff ff ff 	mov    -0xf0(%rbp),%rax
    40001712:	48 8d 50 08          	lea    0x8(%rax),%rdx
    40001716:	48 89 95 10 ff ff ff 	mov    %rdx,-0xf0(%rbp)
    4000171d:	48 8b 00             	mov    (%rax),%rax
    40001720:	48 89 85 40 ff ff ff 	mov    %rax,-0xc0(%rbp)
    40001727:	8b 85 08 ff ff ff    	mov    -0xf8(%rbp),%eax
    4000172d:	83 f8 2f             	cmp    $0x2f,%eax
    40001730:	77 23                	ja     40001755 <syscall+0x148>
    40001732:	48 8b 85 18 ff ff ff 	mov    -0xe8(%rbp),%rax
    40001739:	8b 95 08 ff ff ff    	mov    -0xf8(%rbp),%edx
    4000173f:	89 d2                	mov    %edx,%edx
    40001741:	48 01 d0             	add    %rdx,%rax
    40001744:	8b 95 08 ff ff ff    	mov    -0xf8(%rbp),%edx
    4000174a:	83 c2 08             	add    $0x8,%edx
    4000174d:	89 95 08 ff ff ff    	mov    %edx,-0xf8(%rbp)
    40001753:	eb 12                	jmp    40001767 <syscall+0x15a>
    40001755:	48 8b 85 10 ff ff ff 	mov    -0xf0(%rbp),%rax
    4000175c:	48 8d 50 08          	lea    0x8(%rax),%rdx
    40001760:	48 89 95 10 ff ff ff 	mov    %rdx,-0xf0(%rbp)
    40001767:	48 8b 00             	mov    (%rax),%rax
    4000176a:	48 89 85 38 ff ff ff 	mov    %rax,-0xc8(%rbp)
    40001771:	8b 85 08 ff ff ff    	mov    -0xf8(%rbp),%eax
    40001777:	83 f8 2f             	cmp    $0x2f,%eax
    4000177a:	77 23                	ja     4000179f <syscall+0x192>
    4000177c:	48 8b 85 18 ff ff ff 	mov    -0xe8(%rbp),%rax
    40001783:	8b 95 08 ff ff ff    	mov    -0xf8(%rbp),%edx
    40001789:	89 d2                	mov    %edx,%edx
    4000178b:	48 01 d0             	add    %rdx,%rax
    4000178e:	8b 95 08 ff ff ff    	mov    -0xf8(%rbp),%edx
    40001794:	83 c2 08             	add    $0x8,%edx
    40001797:	89 95 08 ff ff ff    	mov    %edx,-0xf8(%rbp)
    4000179d:	eb 12                	jmp    400017b1 <syscall+0x1a4>
    4000179f:	48 8b 85 10 ff ff ff 	mov    -0xf0(%rbp),%rax
    400017a6:	48 8d 50 08          	lea    0x8(%rax),%rdx
    400017aa:	48 89 95 10 ff ff ff 	mov    %rdx,-0xf0(%rbp)
    400017b1:	48 8b 00             	mov    (%rax),%rax
    400017b4:	48 89 85 30 ff ff ff 	mov    %rax,-0xd0(%rbp)
    400017bb:	8b 85 08 ff ff ff    	mov    -0xf8(%rbp),%eax
    400017c1:	83 f8 2f             	cmp    $0x2f,%eax
    400017c4:	77 23                	ja     400017e9 <syscall+0x1dc>
    400017c6:	48 8b 85 18 ff ff ff 	mov    -0xe8(%rbp),%rax
    400017cd:	8b 95 08 ff ff ff    	mov    -0xf8(%rbp),%edx
    400017d3:	89 d2                	mov    %edx,%edx
    400017d5:	48 01 d0             	add    %rdx,%rax
    400017d8:	8b 95 08 ff ff ff    	mov    -0xf8(%rbp),%edx
    400017de:	83 c2 08             	add    $0x8,%edx
    400017e1:	89 95 08 ff ff ff    	mov    %edx,-0xf8(%rbp)
    400017e7:	eb 12                	jmp    400017fb <syscall+0x1ee>
    400017e9:	48 8b 85 10 ff ff ff 	mov    -0xf0(%rbp),%rax
    400017f0:	48 8d 50 08          	lea    0x8(%rax),%rdx
    400017f4:	48 89 95 10 ff ff ff 	mov    %rdx,-0xf0(%rbp)
    400017fb:	48 8b 00             	mov    (%rax),%rax
    400017fe:	48 89 85 28 ff ff ff 	mov    %rax,-0xd8(%rbp)
    40001805:	8b 85 08 ff ff ff    	mov    -0xf8(%rbp),%eax
    4000180b:	83 f8 2f             	cmp    $0x2f,%eax
    4000180e:	77 23                	ja     40001833 <syscall+0x226>
    40001810:	48 8b 85 18 ff ff ff 	mov    -0xe8(%rbp),%rax
    40001817:	8b 95 08 ff ff ff    	mov    -0xf8(%rbp),%edx
    4000181d:	89 d2                	mov    %edx,%edx
    4000181f:	48 01 d0             	add    %rdx,%rax
    40001822:	8b 95 08 ff ff ff    	mov    -0xf8(%rbp),%edx
    40001828:	83 c2 08             	add    $0x8,%edx
    4000182b:	89 95 08 ff ff ff    	mov    %edx,-0xf8(%rbp)
    40001831:	eb 12                	jmp    40001845 <syscall+0x238>
    40001833:	48 8b 85 10 ff ff ff 	mov    -0xf0(%rbp),%rax
    4000183a:	48 8d 50 08          	lea    0x8(%rax),%rdx
    4000183e:	48 89 95 10 ff ff ff 	mov    %rdx,-0xf0(%rbp)
    40001845:	48 8b 00             	mov    (%rax),%rax
    40001848:	48 89 85 20 ff ff ff 	mov    %rax,-0xe0(%rbp)
    4000184f:	4c 8b 85 28 ff ff ff 	mov    -0xd8(%rbp),%r8
    40001856:	48 8b bd 30 ff ff ff 	mov    -0xd0(%rbp),%rdi
    4000185d:	48 8b 8d 38 ff ff ff 	mov    -0xc8(%rbp),%rcx
    40001864:	48 8b 95 40 ff ff ff 	mov    -0xc0(%rbp),%rdx
    4000186b:	48 8b b5 48 ff ff ff 	mov    -0xb8(%rbp),%rsi
    40001872:	48 8b 85 f8 fe ff ff 	mov    -0x108(%rbp),%rax
    40001879:	48 83 ec 08          	sub    $0x8,%rsp
    4000187d:	ff b5 20 ff ff ff    	push   -0xe0(%rbp)
    40001883:	4d 89 c1             	mov    %r8,%r9
    40001886:	49 89 f8             	mov    %rdi,%r8
    40001889:	48 89 c7             	mov    %rax,%rdi
    4000188c:	e8 06 00 00 00       	call   40001897 <syscall6>
    40001891:	48 83 c4 10          	add    $0x10,%rsp
    40001895:	c9                   	leave  
    40001896:	c3                   	ret    

0000000040001897 <syscall6>:
    40001897:	55                   	push   %rbp
    40001898:	48 89 e5             	mov    %rsp,%rbp
    4000189b:	41 55                	push   %r13
    4000189d:	41 54                	push   %r12
    4000189f:	53                   	push   %rbx
    400018a0:	48 89 7d d0          	mov    %rdi,-0x30(%rbp)
    400018a4:	48 89 75 c8          	mov    %rsi,-0x38(%rbp)
    400018a8:	48 89 55 c0          	mov    %rdx,-0x40(%rbp)
    400018ac:	48 89 4d b8          	mov    %rcx,-0x48(%rbp)
    400018b0:	4c 89 45 b0          	mov    %r8,-0x50(%rbp)
    400018b4:	4c 89 4d a8          	mov    %r9,-0x58(%rbp)
    400018b8:	48 8b 45 d0          	mov    -0x30(%rbp),%rax
    400018bc:	48 8b 7d c8          	mov    -0x38(%rbp),%rdi
    400018c0:	48 8b 75 c0          	mov    -0x40(%rbp),%rsi
    400018c4:	48 8b 55 b8          	mov    -0x48(%rbp),%rdx
    400018c8:	48 8b 5d b0          	mov    -0x50(%rbp),%rbx
    400018cc:	4c 8b 65 a8          	mov    -0x58(%rbp),%r12
    400018d0:	4c 8b 6d 10          	mov    0x10(%rbp),%r13
    400018d4:	49 89 da             	mov    %rbx,%r10
    400018d7:	4d 89 e0             	mov    %r12,%r8
    400018da:	4d 89 e9             	mov    %r13,%r9
    400018dd:	0f 05                	syscall 
    400018df:	48 89 45 e0          	mov    %rax,-0x20(%rbp)
    400018e3:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
    400018e7:	5b                   	pop    %rbx
    400018e8:	41 5c                	pop    %r12
    400018ea:	41 5d                	pop    %r13
    400018ec:	5d                   	pop    %rbp
    400018ed:	c3                   	ret    

00000000400018ee <k_syscall6>:
    400018ee:	55                   	push   %rbp
    400018ef:	48 89 e5             	mov    %rsp,%rbp
    400018f2:	41 55                	push   %r13
    400018f4:	41 54                	push   %r12
    400018f6:	53                   	push   %rbx
    400018f7:	48 89 7d d0          	mov    %rdi,-0x30(%rbp)
    400018fb:	48 89 75 c8          	mov    %rsi,-0x38(%rbp)
    400018ff:	48 89 55 c0          	mov    %rdx,-0x40(%rbp)
    40001903:	48 89 4d b8          	mov    %rcx,-0x48(%rbp)
    40001907:	4c 89 45 b0          	mov    %r8,-0x50(%rbp)
    4000190b:	4c 89 4d a8          	mov    %r9,-0x58(%rbp)
    4000190f:	48 8b 45 d0          	mov    -0x30(%rbp),%rax
    40001913:	48 8b 7d c8          	mov    -0x38(%rbp),%rdi
    40001917:	48 8b 75 c0          	mov    -0x40(%rbp),%rsi
    4000191b:	48 8b 55 b8          	mov    -0x48(%rbp),%rdx
    4000191f:	48 8b 5d b0          	mov    -0x50(%rbp),%rbx
    40001923:	4c 8b 65 a8          	mov    -0x58(%rbp),%r12
    40001927:	4c 8b 6d 10          	mov    0x10(%rbp),%r13
    4000192b:	49 89 da             	mov    %rbx,%r10
    4000192e:	4d 89 e0             	mov    %r12,%r8
    40001931:	4d 89 e9             	mov    %r13,%r9
    40001934:	0f 05                	syscall 
    40001936:	48 89 45 e0          	mov    %rax,-0x20(%rbp)
    4000193a:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
    4000193e:	5b                   	pop    %rbx
    4000193f:	41 5c                	pop    %r12
    40001941:	41 5d                	pop    %r13
    40001943:	5d                   	pop    %rbp
    40001944:	c3                   	ret    

0000000040001945 <raw_syscall6>:
    40001945:	55                   	push   %rbp
    40001946:	48 89 e5             	mov    %rsp,%rbp
    40001949:	48 89 7d e8          	mov    %rdi,-0x18(%rbp)
    4000194d:	48 89 75 e0          	mov    %rsi,-0x20(%rbp)
    40001951:	48 89 55 d8          	mov    %rdx,-0x28(%rbp)
    40001955:	48 89 4d d0          	mov    %rcx,-0x30(%rbp)
    40001959:	4c 89 45 c8          	mov    %r8,-0x38(%rbp)
    4000195d:	4c 89 4d c0          	mov    %r9,-0x40(%rbp)
    40001961:	48 8b 45 e8          	mov    -0x18(%rbp),%rax
    40001965:	48 8b 7d e0          	mov    -0x20(%rbp),%rdi
    40001969:	48 8b 75 d8          	mov    -0x28(%rbp),%rsi
    4000196d:	48 8b 55 d0          	mov    -0x30(%rbp),%rdx
    40001971:	4c 8b 55 c8          	mov    -0x38(%rbp),%r10
    40001975:	4c 8b 45 c0          	mov    -0x40(%rbp),%r8
    40001979:	4c 8b 4d 10          	mov    0x10(%rbp),%r9
    4000197d:	0f 05                	syscall 
    4000197f:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    40001983:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    40001987:	5d                   	pop    %rbp
    40001988:	c3                   	ret    

0000000040001989 <klog>:
    40001989:	55                   	push   %rbp
    4000198a:	48 89 e5             	mov    %rsp,%rbp
    4000198d:	48 83 ec 10          	sub    $0x10,%rsp
    40001991:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
    40001995:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    40001999:	48 83 ec 08          	sub    $0x8,%rsp
    4000199d:	6a 00                	push   $0x0
    4000199f:	41 b9 00 00 00 00    	mov    $0x0,%r9d
    400019a5:	41 b8 00 00 00 00    	mov    $0x0,%r8d
    400019ab:	b9 00 00 00 00       	mov    $0x0,%ecx
    400019b0:	ba 00 00 00 00       	mov    $0x0,%edx
    400019b5:	48 89 c6             	mov    %rax,%rsi
    400019b8:	bf 00 10 00 00       	mov    $0x1000,%edi
    400019bd:	e8 d5 fe ff ff       	call   40001897 <syscall6>
    400019c2:	48 83 c4 10          	add    $0x10,%rsp
    400019c6:	90                   	nop
    400019c7:	c9                   	leave  
    400019c8:	c3                   	ret    
    400019c9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

00000000400019d0 <dummy>:
    400019d0:	c3                   	ret    
    400019d1:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    400019d8:	00 00 00 
    400019db:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

00000000400019e0 <__init_ssp>:
    400019e0:	c3                   	ret    
    400019e1:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    400019e8:	00 00 00 
    400019eb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

00000000400019f0 <__init_libc>:
    400019f0:	48 81 ec 58 01 00 00 	sub    $0x158,%rsp
    400019f7:	48 89 fa             	mov    %rdi,%rdx
    400019fa:	31 c0                	xor    %eax,%eax
    400019fc:	b9 26 00 00 00       	mov    $0x26,%ecx
    40001a01:	4c 8d 44 24 20       	lea    0x20(%rsp),%r8
    40001a06:	48 89 15 1b 68 00 00 	mov    %rdx,0x681b(%rip)        # 40008228 <__environ>
    40001a0d:	48 83 3a 00          	cmpq   $0x0,(%rdx)
    40001a11:	4c 89 c7             	mov    %r8,%rdi
    40001a14:	f3 48 ab             	rep stos %rax,%es:(%rdi)
    40001a17:	0f 84 b3 01 00 00    	je     40001bd0 <__init_libc+0x1e0>
    40001a1d:	0f 1f 00             	nopl   (%rax)
    40001a20:	48 89 c1             	mov    %rax,%rcx
    40001a23:	48 83 c0 01          	add    $0x1,%rax
    40001a27:	48 83 3c c2 00       	cmpq   $0x0,(%rdx,%rax,8)
    40001a2c:	75 f2                	jne    40001a20 <__init_libc+0x30>
    40001a2e:	48 8d 04 cd 10 00 00 	lea    0x10(,%rcx,8),%rax
    40001a35:	00 
    40001a36:	48 01 d0             	add    %rdx,%rax
    40001a39:	48 89 05 68 63 00 00 	mov    %rax,0x6368(%rip)        # 40007da8 <__libc+0x8>
    40001a40:	48 8b 10             	mov    (%rax),%rdx
    40001a43:	48 83 c0 08          	add    $0x8,%rax
    40001a47:	48 85 d2             	test   %rdx,%rdx
    40001a4a:	0f 84 90 01 00 00    	je     40001be0 <__init_libc+0x1f0>
    40001a50:	48 83 fa 25          	cmp    $0x25,%rdx
    40001a54:	77 08                	ja     40001a5e <__init_libc+0x6e>
    40001a56:	48 8b 08             	mov    (%rax),%rcx
    40001a59:	48 89 4c d4 20       	mov    %rcx,0x20(%rsp,%rdx,8)
    40001a5e:	48 8b 50 08          	mov    0x8(%rax),%rdx
    40001a62:	48 83 c0 10          	add    $0x10,%rax
    40001a66:	48 85 d2             	test   %rdx,%rdx
    40001a69:	75 e5                	jne    40001a50 <__init_libc+0x60>
    40001a6b:	48 8b 8c 24 a0 00 00 	mov    0xa0(%rsp),%rcx
    40001a72:	00 
    40001a73:	48 8b 84 24 20 01 00 	mov    0x120(%rsp),%rax
    40001a7a:	00 
    40001a7b:	48 8b 54 24 50       	mov    0x50(%rsp),%rdx
    40001a80:	48 89 0d 11 63 00 00 	mov    %rcx,0x6311(%rip)        # 40007d98 <__hwcap>
    40001a87:	48 85 c0             	test   %rax,%rax
    40001a8a:	74 07                	je     40001a93 <__init_libc+0xa3>
    40001a8c:	48 89 05 ed 62 00 00 	mov    %rax,0x62ed(%rip)        # 40007d80 <__sysinfo>
    40001a93:	48 89 15 36 63 00 00 	mov    %rdx,0x6336(%rip)        # 40007dd0 <__libc+0x30>
    40001a9a:	48 85 f6             	test   %rsi,%rsi
    40001a9d:	0f 84 fd 00 00 00    	je     40001ba0 <__init_libc+0x1b0>
    40001aa3:	48 89 35 de 62 00 00 	mov    %rsi,0x62de(%rip)        # 40007d88 <__progname_full>
    40001aaa:	48 89 35 df 62 00 00 	mov    %rsi,0x62df(%rip)        # 40007d90 <__progname>
    40001ab1:	0f b6 16             	movzbl (%rsi),%edx
    40001ab4:	84 d2                	test   %dl,%dl
    40001ab6:	74 1f                	je     40001ad7 <__init_libc+0xe7>
    40001ab8:	48 8d 46 01          	lea    0x1(%rsi),%rax
    40001abc:	0f 1f 40 00          	nopl   0x0(%rax)
    40001ac0:	80 fa 2f             	cmp    $0x2f,%dl
    40001ac3:	75 07                	jne    40001acc <__init_libc+0xdc>
    40001ac5:	48 89 05 c4 62 00 00 	mov    %rax,0x62c4(%rip)        # 40007d90 <__progname>
    40001acc:	0f b6 10             	movzbl (%rax),%edx
    40001acf:	48 83 c0 01          	add    $0x1,%rax
    40001ad3:	84 d2                	test   %dl,%dl
    40001ad5:	75 e9                	jne    40001ac0 <__init_libc+0xd0>
    40001ad7:	4c 89 c7             	mov    %r8,%rdi
    40001ada:	e8 51 33 00 00       	call   40004e30 <__init_tls>
    40001adf:	48 8b bc 24 e8 00 00 	mov    0xe8(%rsp),%rdi
    40001ae6:	00 
    40001ae7:	e8 f4 fe ff ff       	call   400019e0 <__init_ssp>
    40001aec:	48 8b 84 24 80 00 00 	mov    0x80(%rsp),%rax
    40001af3:	00 
    40001af4:	48 39 44 24 78       	cmp    %rax,0x78(%rsp)
    40001af9:	74 7d                	je     40001b78 <__init_libc+0x188>
    40001afb:	66 0f ef c0          	pxor   %xmm0,%xmm0
    40001aff:	48 89 e7             	mov    %rsp,%rdi
    40001b02:	b8 07 00 00 00       	mov    $0x7,%eax
    40001b07:	31 d2                	xor    %edx,%edx
    40001b09:	48 c7 44 24 10 00 00 	movq   $0x0,0x10(%rsp)
    40001b10:	00 00 
    40001b12:	be 03 00 00 00       	mov    $0x3,%esi
    40001b17:	0f 29 04 24          	movaps %xmm0,(%rsp)
    40001b1b:	c7 44 24 10 02 00 00 	movl   $0x2,0x10(%rsp)
    40001b22:	00 
    40001b23:	c7 44 24 08 01 00 00 	movl   $0x1,0x8(%rsp)
    40001b2a:	00 
    40001b2b:	0f 05                	syscall 
    40001b2d:	85 c0                	test   %eax,%eax
    40001b2f:	79 01                	jns    40001b32 <__init_libc+0x142>
    40001b31:	f4                   	hlt    
    40001b32:	48 89 fa             	mov    %rdi,%rdx
    40001b35:	4c 8d 44 24 18       	lea    0x18(%rsp),%r8
    40001b3a:	41 b9 02 00 00 00    	mov    $0x2,%r9d
    40001b40:	bf a5 6b 00 40       	mov    $0x40006ba5,%edi
    40001b45:	be 02 80 00 00       	mov    $0x8002,%esi
    40001b4a:	f6 42 06 20          	testb  $0x20,0x6(%rdx)
    40001b4e:	74 0b                	je     40001b5b <__init_libc+0x16b>
    40001b50:	4c 89 c8             	mov    %r9,%rax
    40001b53:	0f 05                	syscall 
    40001b55:	48 85 c0             	test   %rax,%rax
    40001b58:	79 01                	jns    40001b5b <__init_libc+0x16b>
    40001b5a:	f4                   	hlt    
    40001b5b:	48 83 c2 08          	add    $0x8,%rdx
    40001b5f:	4c 39 c2             	cmp    %r8,%rdx
    40001b62:	75 e6                	jne    40001b4a <__init_libc+0x15a>
    40001b64:	c6 05 37 62 00 00 01 	movb   $0x1,0x6237(%rip)        # 40007da2 <__libc+0x2>
    40001b6b:	48 81 c4 58 01 00 00 	add    $0x158,%rsp
    40001b72:	c3                   	ret    
    40001b73:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40001b78:	48 8b 84 24 90 00 00 	mov    0x90(%rsp),%rax
    40001b7f:	00 
    40001b80:	48 39 84 24 88 00 00 	cmp    %rax,0x88(%rsp)
    40001b87:	00 
    40001b88:	0f 85 6d ff ff ff    	jne    40001afb <__init_libc+0x10b>
    40001b8e:	48 83 bc 24 d8 00 00 	cmpq   $0x0,0xd8(%rsp)
    40001b95:	00 00 
    40001b97:	0f 85 5e ff ff ff    	jne    40001afb <__init_libc+0x10b>
    40001b9d:	eb cc                	jmp    40001b6b <__init_libc+0x17b>
    40001b9f:	90                   	nop
    40001ba0:	48 8b 84 24 18 01 00 	mov    0x118(%rsp),%rax
    40001ba7:	00 
    40001ba8:	48 85 c0             	test   %rax,%rax
    40001bab:	75 43                	jne    40001bf0 <__init_libc+0x200>
    40001bad:	48 c7 05 d0 61 00 00 	movq   $0x40006c7d,0x61d0(%rip)        # 40007d88 <__progname_full>
    40001bb4:	7d 6c 00 40 
    40001bb8:	48 c7 05 cd 61 00 00 	movq   $0x40006c7d,0x61cd(%rip)        # 40007d90 <__progname>
    40001bbf:	7d 6c 00 40 
    40001bc3:	e9 0f ff ff ff       	jmp    40001ad7 <__init_libc+0xe7>
    40001bc8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    40001bcf:	00 
    40001bd0:	b8 08 00 00 00       	mov    $0x8,%eax
    40001bd5:	e9 5c fe ff ff       	jmp    40001a36 <__init_libc+0x46>
    40001bda:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40001be0:	48 c7 05 ad 61 00 00 	movq   $0x0,0x61ad(%rip)        # 40007d98 <__hwcap>
    40001be7:	00 00 00 00 
    40001beb:	e9 a3 fe ff ff       	jmp    40001a93 <__init_libc+0xa3>
    40001bf0:	48 89 c6             	mov    %rax,%rsi
    40001bf3:	e9 ab fe ff ff       	jmp    40001aa3 <__init_libc+0xb3>
    40001bf8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    40001bff:	00 

0000000040001c00 <__libc_start_init>:
    40001c00:	53                   	push   %rbx
    40001c01:	bb 00 00 00 00       	mov    $0x0,%ebx
    40001c06:	e8 59 45 00 00       	call   40006164 <_init>
    40001c0b:	48 81 fb 00 00 00 00 	cmp    $0x0,%rbx
    40001c12:	73 13                	jae    40001c27 <__libc_start_init+0x27>
    40001c14:	0f 1f 40 00          	nopl   0x0(%rax)
    40001c18:	ff 13                	call   *(%rbx)
    40001c1a:	48 83 c3 08          	add    $0x8,%rbx
    40001c1e:	48 81 fb 00 00 00 00 	cmp    $0x0,%rbx
    40001c25:	72 f1                	jb     40001c18 <__libc_start_init+0x18>
    40001c27:	5b                   	pop    %rbx
    40001c28:	c3                   	ret    
    40001c29:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000040001c30 <libc_start_main_stage2>:
    40001c30:	41 55                	push   %r13
    40001c32:	48 63 c6             	movslq %esi,%rax
    40001c35:	41 54                	push   %r12
    40001c37:	4c 8d 6c c2 08       	lea    0x8(%rdx,%rax,8),%r13
    40001c3c:	49 89 d4             	mov    %rdx,%r12
    40001c3f:	55                   	push   %rbp
    40001c40:	48 89 c5             	mov    %rax,%rbp
    40001c43:	53                   	push   %rbx
    40001c44:	48 89 fb             	mov    %rdi,%rbx
    40001c47:	48 83 ec 08          	sub    $0x8,%rsp
    40001c4b:	e8 b0 ff ff ff       	call   40001c00 <__libc_start_init>
    40001c50:	89 ef                	mov    %ebp,%edi
    40001c52:	4c 89 ea             	mov    %r13,%rdx
    40001c55:	4c 89 e6             	mov    %r12,%rsi
    40001c58:	ff d3                	call   *%rbx
    40001c5a:	89 c7                	mov    %eax,%edi
    40001c5c:	e8 8f 00 00 00       	call   40001cf0 <exit>
    40001c61:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40001c68:	00 00 00 
    40001c6b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040001c70 <__libc_start_main>:
    40001c70:	41 55                	push   %r13
    40001c72:	48 63 c6             	movslq %esi,%rax
    40001c75:	49 89 fd             	mov    %rdi,%r13
    40001c78:	41 54                	push   %r12
    40001c7a:	48 8d 7c c2 08       	lea    0x8(%rdx,%rax,8),%rdi
    40001c7f:	49 89 d4             	mov    %rdx,%r12
    40001c82:	55                   	push   %rbp
    40001c83:	48 8b 32             	mov    (%rdx),%rsi
    40001c86:	48 89 c5             	mov    %rax,%rbp
    40001c89:	e8 62 fd ff ff       	call   400019f0 <__init_libc>
    40001c8e:	b8 30 1c 00 40       	mov    $0x40001c30,%eax
    40001c93:	4c 89 e2             	mov    %r12,%rdx
    40001c96:	89 ee                	mov    %ebp,%esi
    40001c98:	4c 89 ef             	mov    %r13,%rdi
    40001c9b:	5d                   	pop    %rbp
    40001c9c:	41 5c                	pop    %r12
    40001c9e:	41 5d                	pop    %r13
    40001ca0:	ff e0                	jmp    *%rax
    40001ca2:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40001ca9:	00 00 00 
    40001cac:	0f 1f 40 00          	nopl   0x0(%rax)

0000000040001cb0 <__funcs_on_exit>:
    40001cb0:	c3                   	ret    
    40001cb1:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40001cb8:	00 00 00 
    40001cbb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040001cc0 <__libc_exit_fini>:
    40001cc0:	53                   	push   %rbx
    40001cc1:	bb 00 00 00 00       	mov    $0x0,%ebx
    40001cc6:	48 81 fb 00 00 00 00 	cmp    $0x0,%rbx
    40001ccd:	76 12                	jbe    40001ce1 <__libc_exit_fini+0x21>
    40001ccf:	90                   	nop
    40001cd0:	48 83 eb 08          	sub    $0x8,%rbx
    40001cd4:	31 c0                	xor    %eax,%eax
    40001cd6:	ff 13                	call   *(%rbx)
    40001cd8:	48 81 fb 00 00 00 00 	cmp    $0x0,%rbx
    40001cdf:	77 ef                	ja     40001cd0 <__libc_exit_fini+0x10>
    40001ce1:	31 c0                	xor    %eax,%eax
    40001ce3:	5b                   	pop    %rbx
    40001ce4:	e9 7e 44 00 00       	jmp    40006167 <_fini>
    40001ce9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000040001cf0 <exit>:
    40001cf0:	55                   	push   %rbp
    40001cf1:	89 fd                	mov    %edi,%ebp
    40001cf3:	e8 b8 ff ff ff       	call   40001cb0 <__funcs_on_exit>
    40001cf8:	e8 c3 ff ff ff       	call   40001cc0 <__libc_exit_fini>
    40001cfd:	31 c0                	xor    %eax,%eax
    40001cff:	e8 9c 41 00 00       	call   40005ea0 <__stdio_exit>
    40001d04:	89 ef                	mov    %ebp,%edi
    40001d06:	e8 b5 33 00 00       	call   400050c0 <_Exit>
    40001d0b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040001d10 <fcntl>:
    40001d10:	48 83 ec 58          	sub    $0x58,%rsp
    40001d14:	49 89 d2             	mov    %rdx,%r10
    40001d17:	48 63 ff             	movslq %edi,%rdi
    40001d1a:	48 8d 4c 24 60       	lea    0x60(%rsp),%rcx
    40001d1f:	48 89 54 24 30       	mov    %rdx,0x30(%rsp)
    40001d24:	48 89 4c 24 10       	mov    %rcx,0x10(%rsp)
    40001d29:	48 8d 4c 24 20       	lea    0x20(%rsp),%rcx
    40001d2e:	c7 44 24 08 10 00 00 	movl   $0x10,0x8(%rsp)
    40001d35:	00 
    40001d36:	48 89 4c 24 18       	mov    %rcx,0x18(%rsp)
    40001d3b:	83 fe 04             	cmp    $0x4,%esi
    40001d3e:	0f 84 9c 00 00 00    	je     40001de0 <fcntl+0xd0>
    40001d44:	83 fe 07             	cmp    $0x7,%esi
    40001d47:	0f 84 fb 00 00 00    	je     40001e48 <fcntl+0x138>
    40001d4d:	83 fe 09             	cmp    $0x9,%esi
    40001d50:	74 5e                	je     40001db0 <fcntl+0xa0>
    40001d52:	81 fe 06 04 00 00    	cmp    $0x406,%esi
    40001d58:	0f 85 aa 00 00 00    	jne    40001e08 <fcntl+0xf8>
    40001d5e:	41 b8 48 00 00 00    	mov    $0x48,%r8d
    40001d64:	be 06 04 00 00       	mov    $0x406,%esi
    40001d69:	4c 89 c0             	mov    %r8,%rax
    40001d6c:	0f 05                	syscall 
    40001d6e:	83 f8 ea             	cmp    $0xffffffea,%eax
    40001d71:	0f 85 b4 00 00 00    	jne    40001e2b <fcntl+0x11b>
    40001d77:	45 31 c9             	xor    %r9d,%r9d
    40001d7a:	4c 89 c0             	mov    %r8,%rax
    40001d7d:	4c 89 ca             	mov    %r9,%rdx
    40001d80:	0f 05                	syscall 
    40001d82:	83 f8 ea             	cmp    $0xffffffea,%eax
    40001d85:	0f 84 95 00 00 00    	je     40001e20 <fcntl+0x110>
    40001d8b:	85 c0                	test   %eax,%eax
    40001d8d:	78 0a                	js     40001d99 <fcntl+0x89>
    40001d8f:	48 63 f8             	movslq %eax,%rdi
    40001d92:	b8 03 00 00 00       	mov    $0x3,%eax
    40001d97:	0f 05                	syscall 
    40001d99:	48 c7 c7 ea ff ff ff 	mov    $0xffffffffffffffea,%rdi
    40001da0:	e8 9b 01 00 00       	call   40001f40 <__syscall_ret>
    40001da5:	eb 57                	jmp    40001dfe <fcntl+0xee>
    40001da7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    40001dae:	00 00 
    40001db0:	41 b8 48 00 00 00    	mov    $0x48,%r8d
    40001db6:	48 89 e2             	mov    %rsp,%rdx
    40001db9:	be 10 00 00 00       	mov    $0x10,%esi
    40001dbe:	4c 89 c0             	mov    %r8,%rax
    40001dc1:	0f 05                	syscall 
    40001dc3:	83 f8 ea             	cmp    $0xffffffea,%eax
    40001dc6:	0f 84 b4 00 00 00    	je     40001e80 <fcntl+0x170>
    40001dcc:	85 c0                	test   %eax,%eax
    40001dce:	75 40                	jne    40001e10 <fcntl+0x100>
    40001dd0:	83 3c 24 02          	cmpl   $0x2,(%rsp)
    40001dd4:	8b 44 24 04          	mov    0x4(%rsp),%eax
    40001dd8:	75 24                	jne    40001dfe <fcntl+0xee>
    40001dda:	f7 d8                	neg    %eax
    40001ddc:	eb 20                	jmp    40001dfe <fcntl+0xee>
    40001dde:	66 90                	xchg   %ax,%ax
    40001de0:	49 81 ca 00 80 00 00 	or     $0x8000,%r10
    40001de7:	be 04 00 00 00       	mov    $0x4,%esi
    40001dec:	b8 48 00 00 00       	mov    $0x48,%eax
    40001df1:	4c 89 d2             	mov    %r10,%rdx
    40001df4:	0f 05                	syscall 
    40001df6:	48 89 c7             	mov    %rax,%rdi
    40001df9:	e8 42 01 00 00       	call   40001f40 <__syscall_ret>
    40001dfe:	48 83 c4 58          	add    $0x58,%rsp
    40001e02:	c3                   	ret    
    40001e03:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40001e08:	48 63 f6             	movslq %esi,%rsi
    40001e0b:	eb df                	jmp    40001dec <fcntl+0xdc>
    40001e0d:	0f 1f 00             	nopl   (%rax)
    40001e10:	48 63 f8             	movslq %eax,%rdi
    40001e13:	e8 28 01 00 00       	call   40001f40 <__syscall_ret>
    40001e18:	eb e4                	jmp    40001dfe <fcntl+0xee>
    40001e1a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40001e20:	4c 89 c0             	mov    %r8,%rax
    40001e23:	4c 89 ce             	mov    %r9,%rsi
    40001e26:	4c 89 d2             	mov    %r10,%rdx
    40001e29:	0f 05                	syscall 
    40001e2b:	48 63 f8             	movslq %eax,%rdi
    40001e2e:	85 c0                	test   %eax,%eax
    40001e30:	78 c7                	js     40001df9 <fcntl+0xe9>
    40001e32:	be 02 00 00 00       	mov    $0x2,%esi
    40001e37:	ba 01 00 00 00       	mov    $0x1,%edx
    40001e3c:	4c 89 c0             	mov    %r8,%rax
    40001e3f:	0f 05                	syscall 
    40001e41:	eb b6                	jmp    40001df9 <fcntl+0xe9>
    40001e43:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40001e48:	48 83 ec 08          	sub    $0x8,%rsp
    40001e4c:	48 89 d1             	mov    %rdx,%rcx
    40001e4f:	48 89 fe             	mov    %rdi,%rsi
    40001e52:	ba 07 00 00 00       	mov    $0x7,%edx
    40001e57:	6a 00                	push   $0x0
    40001e59:	45 31 c9             	xor    %r9d,%r9d
    40001e5c:	45 31 c0             	xor    %r8d,%r8d
    40001e5f:	bf 48 00 00 00       	mov    $0x48,%edi
    40001e64:	e8 51 f7 ff ff       	call   400015ba <__syscall_cp>
    40001e69:	48 89 c7             	mov    %rax,%rdi
    40001e6c:	e8 cf 00 00 00       	call   40001f40 <__syscall_ret>
    40001e71:	5a                   	pop    %rdx
    40001e72:	59                   	pop    %rcx
    40001e73:	48 83 c4 58          	add    $0x58,%rsp
    40001e77:	c3                   	ret    
    40001e78:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    40001e7f:	00 
    40001e80:	be 09 00 00 00       	mov    $0x9,%esi
    40001e85:	4c 89 c0             	mov    %r8,%rax
    40001e88:	4c 89 d2             	mov    %r10,%rdx
    40001e8b:	0f 05                	syscall 
    40001e8d:	e9 6c ff ff ff       	jmp    40001dfe <fcntl+0xee>
    40001e92:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40001e99:	00 00 00 
    40001e9c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000040001ea0 <open>:
    40001ea0:	53                   	push   %rbx
    40001ea1:	89 f3                	mov    %esi,%ebx
    40001ea3:	49 89 fa             	mov    %rdi,%r10
    40001ea6:	48 83 ec 50          	sub    $0x50,%rsp
    40001eaa:	83 e6 40             	and    $0x40,%esi
    40001ead:	48 89 54 24 30       	mov    %rdx,0x30(%rsp)
    40001eb2:	75 64                	jne    40001f18 <open+0x78>
    40001eb4:	89 d8                	mov    %ebx,%eax
    40001eb6:	31 c9                	xor    %ecx,%ecx
    40001eb8:	25 00 00 41 00       	and    $0x410000,%eax
    40001ebd:	3d 00 00 41 00       	cmp    $0x410000,%eax
    40001ec2:	74 54                	je     40001f18 <open+0x78>
    40001ec4:	48 83 ec 08          	sub    $0x8,%rsp
    40001ec8:	89 da                	mov    %ebx,%edx
    40001eca:	bf 02 00 00 00       	mov    $0x2,%edi
    40001ecf:	45 31 c9             	xor    %r9d,%r9d
    40001ed2:	6a 00                	push   $0x0
    40001ed4:	80 ce 80             	or     $0x80,%dh
    40001ed7:	45 31 c0             	xor    %r8d,%r8d
    40001eda:	4c 89 d6             	mov    %r10,%rsi
    40001edd:	48 63 d2             	movslq %edx,%rdx
    40001ee0:	e8 d5 f6 ff ff       	call   400015ba <__syscall_cp>
    40001ee5:	5a                   	pop    %rdx
    40001ee6:	59                   	pop    %rcx
    40001ee7:	48 63 f8             	movslq %eax,%rdi
    40001eea:	85 c0                	test   %eax,%eax
    40001eec:	78 19                	js     40001f07 <open+0x67>
    40001eee:	81 e3 00 00 08 00    	and    $0x80000,%ebx
    40001ef4:	74 11                	je     40001f07 <open+0x67>
    40001ef6:	b8 48 00 00 00       	mov    $0x48,%eax
    40001efb:	be 02 00 00 00       	mov    $0x2,%esi
    40001f00:	ba 01 00 00 00       	mov    $0x1,%edx
    40001f05:	0f 05                	syscall 
    40001f07:	e8 34 00 00 00       	call   40001f40 <__syscall_ret>
    40001f0c:	48 83 c4 50          	add    $0x50,%rsp
    40001f10:	5b                   	pop    %rbx
    40001f11:	c3                   	ret    
    40001f12:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40001f18:	48 8d 44 24 60       	lea    0x60(%rsp),%rax
    40001f1d:	c7 44 24 08 10 00 00 	movl   $0x10,0x8(%rsp)
    40001f24:	00 
    40001f25:	8b 4c 24 30          	mov    0x30(%rsp),%ecx
    40001f29:	48 89 44 24 10       	mov    %rax,0x10(%rsp)
    40001f2e:	48 8d 44 24 20       	lea    0x20(%rsp),%rax
    40001f33:	48 89 44 24 18       	mov    %rax,0x18(%rsp)
    40001f38:	eb 8a                	jmp    40001ec4 <open+0x24>
    40001f3a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

0000000040001f40 <__syscall_ret>:
    40001f40:	48 81 ff 00 f0 ff ff 	cmp    $0xfffffffffffff000,%rdi
    40001f47:	77 07                	ja     40001f50 <__syscall_ret+0x10>
    40001f49:	48 89 f8             	mov    %rdi,%rax
    40001f4c:	c3                   	ret    
    40001f4d:	0f 1f 00             	nopl   (%rax)
    40001f50:	48 83 ec 18          	sub    $0x18,%rsp
    40001f54:	48 89 7c 24 08       	mov    %rdi,0x8(%rsp)
    40001f59:	e8 e2 30 00 00       	call   40005040 <__errno_location>
    40001f5e:	48 8b 7c 24 08       	mov    0x8(%rsp),%rdi
    40001f63:	f7 df                	neg    %edi
    40001f65:	89 38                	mov    %edi,(%rax)
    40001f67:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    40001f6e:	48 83 c4 18          	add    $0x18,%rsp
    40001f72:	c3                   	ret    
    40001f73:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40001f7a:	00 00 00 
    40001f7d:	0f 1f 00             	nopl   (%rax)

0000000040001f80 <getdents>:
    40001f80:	b8 ff ff ff 7f       	mov    $0x7fffffff,%eax
    40001f85:	48 83 ec 08          	sub    $0x8,%rsp
    40001f89:	48 63 ff             	movslq %edi,%rdi
    40001f8c:	48 39 c2             	cmp    %rax,%rdx
    40001f8f:	48 0f 47 d0          	cmova  %rax,%rdx
    40001f93:	b8 d9 00 00 00       	mov    $0xd9,%eax
    40001f98:	0f 05                	syscall 
    40001f9a:	48 89 c7             	mov    %rax,%rdi
    40001f9d:	e8 9e ff ff ff       	call   40001f40 <__syscall_ret>
    40001fa2:	48 83 c4 08          	add    $0x8,%rsp
    40001fa6:	c3                   	ret    
    40001fa7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    40001fae:	00 00 

0000000040001fb0 <memfd_create>:
    40001fb0:	48 83 ec 08          	sub    $0x8,%rsp
    40001fb4:	89 f6                	mov    %esi,%esi
    40001fb6:	b8 3f 01 00 00       	mov    $0x13f,%eax
    40001fbb:	0f 05                	syscall 
    40001fbd:	48 89 c7             	mov    %rax,%rdi
    40001fc0:	e8 7b ff ff ff       	call   40001f40 <__syscall_ret>
    40001fc5:	48 83 c4 08          	add    $0x8,%rsp
    40001fc9:	c3                   	ret    
    40001fca:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

0000000040001fd0 <__vm_wait>:
    40001fd0:	c3                   	ret    
    40001fd1:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40001fd8:	00 00 00 
    40001fdb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040001fe0 <__mmap>:
    40001fe0:	41 54                	push   %r12
    40001fe2:	53                   	push   %rbx
    40001fe3:	48 83 ec 28          	sub    $0x28,%rsp
    40001fe7:	41 f7 c1 ff 0f 00 00 	test   $0xfff,%r9d
    40001fee:	0f 85 7c 00 00 00    	jne    40002070 <__mmap+0x90>
    40001ff4:	48 b8 fe ff ff ff ff 	movabs $0x7ffffffffffffffe,%rax
    40001ffb:	ff ff 7f 
    40001ffe:	48 39 c6             	cmp    %rax,%rsi
    40002001:	77 4d                	ja     40002050 <__mmap+0x70>
    40002003:	49 89 fc             	mov    %rdi,%r12
    40002006:	89 cb                	mov    %ecx,%ebx
    40002008:	f6 c1 10             	test   $0x10,%cl
    4000200b:	0f 85 7f 00 00 00    	jne    40002090 <__mmap+0xb0>
    40002011:	48 63 d2             	movslq %edx,%rdx
    40002014:	4c 63 d3             	movslq %ebx,%r10
    40002017:	4d 63 c0             	movslq %r8d,%r8
    4000201a:	b8 09 00 00 00       	mov    $0x9,%eax
    4000201f:	4c 89 e7             	mov    %r12,%rdi
    40002022:	0f 05                	syscall 
    40002024:	48 89 c7             	mov    %rax,%rdi
    40002027:	48 83 f8 ff          	cmp    $0xffffffffffffffff,%rax
    4000202b:	75 16                	jne    40002043 <__mmap+0x63>
    4000202d:	4d 85 e4             	test   %r12,%r12
    40002030:	75 11                	jne    40002043 <__mmap+0x63>
    40002032:	83 e3 30             	and    $0x30,%ebx
    40002035:	48 c7 c0 f4 ff ff ff 	mov    $0xfffffffffffffff4,%rax
    4000203c:	83 fb 20             	cmp    $0x20,%ebx
    4000203f:	48 0f 44 f8          	cmove  %rax,%rdi
    40002043:	48 83 c4 28          	add    $0x28,%rsp
    40002047:	5b                   	pop    %rbx
    40002048:	41 5c                	pop    %r12
    4000204a:	e9 f1 fe ff ff       	jmp    40001f40 <__syscall_ret>
    4000204f:	90                   	nop
    40002050:	e8 eb 2f 00 00       	call   40005040 <__errno_location>
    40002055:	c7 00 0c 00 00 00    	movl   $0xc,(%rax)
    4000205b:	48 83 c4 28          	add    $0x28,%rsp
    4000205f:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    40002066:	5b                   	pop    %rbx
    40002067:	41 5c                	pop    %r12
    40002069:	c3                   	ret    
    4000206a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40002070:	e8 cb 2f 00 00       	call   40005040 <__errno_location>
    40002075:	c7 00 16 00 00 00    	movl   $0x16,(%rax)
    4000207b:	48 83 c4 28          	add    $0x28,%rsp
    4000207f:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    40002086:	5b                   	pop    %rbx
    40002087:	41 5c                	pop    %r12
    40002089:	c3                   	ret    
    4000208a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40002090:	4c 89 4c 24 18       	mov    %r9,0x18(%rsp)
    40002095:	44 89 44 24 14       	mov    %r8d,0x14(%rsp)
    4000209a:	89 54 24 10          	mov    %edx,0x10(%rsp)
    4000209e:	48 89 74 24 08       	mov    %rsi,0x8(%rsp)
    400020a3:	e8 28 ff ff ff       	call   40001fd0 <__vm_wait>
    400020a8:	4c 8b 4c 24 18       	mov    0x18(%rsp),%r9
    400020ad:	44 8b 44 24 14       	mov    0x14(%rsp),%r8d
    400020b2:	8b 54 24 10          	mov    0x10(%rsp),%edx
    400020b6:	48 8b 74 24 08       	mov    0x8(%rsp),%rsi
    400020bb:	e9 51 ff ff ff       	jmp    40002011 <__mmap+0x31>

00000000400020c0 <__mprotect>:
    400020c0:	48 89 f8             	mov    %rdi,%rax
    400020c3:	48 81 e7 00 f0 ff ff 	and    $0xfffffffffffff000,%rdi
    400020ca:	48 83 ec 08          	sub    $0x8,%rsp
    400020ce:	48 63 d2             	movslq %edx,%rdx
    400020d1:	48 8d b4 30 ff 0f 00 	lea    0xfff(%rax,%rsi,1),%rsi
    400020d8:	00 
    400020d9:	b8 0a 00 00 00       	mov    $0xa,%eax
    400020de:	48 81 e6 00 f0 ff ff 	and    $0xfffffffffffff000,%rsi
    400020e5:	48 29 fe             	sub    %rdi,%rsi
    400020e8:	0f 05                	syscall 
    400020ea:	48 89 c7             	mov    %rax,%rdi
    400020ed:	e8 4e fe ff ff       	call   40001f40 <__syscall_ret>
    400020f2:	48 83 c4 08          	add    $0x8,%rsp
    400020f6:	c3                   	ret    
    400020f7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    400020fe:	00 00 

0000000040002100 <dummy>:
    40002100:	c3                   	ret    
    40002101:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40002108:	00 00 00 
    4000210b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040002110 <__munmap>:
    40002110:	55                   	push   %rbp
    40002111:	48 89 f5             	mov    %rsi,%rbp
    40002114:	53                   	push   %rbx
    40002115:	48 89 fb             	mov    %rdi,%rbx
    40002118:	48 83 ec 08          	sub    $0x8,%rsp
    4000211c:	e8 af fe ff ff       	call   40001fd0 <__vm_wait>
    40002121:	b8 0b 00 00 00       	mov    $0xb,%eax
    40002126:	48 89 df             	mov    %rbx,%rdi
    40002129:	48 89 ee             	mov    %rbp,%rsi
    4000212c:	0f 05                	syscall 
    4000212e:	48 89 c7             	mov    %rax,%rdi
    40002131:	e8 0a fe ff ff       	call   40001f40 <__syscall_ret>
    40002136:	48 83 c4 08          	add    $0x8,%rsp
    4000213a:	5b                   	pop    %rbx
    4000213b:	5d                   	pop    %rbp
    4000213c:	c3                   	ret    
    4000213d:	0f 1f 00             	nopl   (%rax)

0000000040002140 <__convert_scm_timestamps>:
    40002140:	c3                   	ret    
    40002141:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40002148:	00 00 00 
    4000214b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040002150 <recvmsg>:
    40002150:	53                   	push   %rbx
    40002151:	48 63 ca             	movslq %edx,%rcx
    40002154:	48 89 f3             	mov    %rsi,%rbx
    40002157:	45 31 c9             	xor    %r9d,%r9d
    4000215a:	45 31 c0             	xor    %r8d,%r8d
    4000215d:	48 83 ec 48          	sub    $0x48,%rsp
    40002161:	f3 0f 6f 06          	movdqu (%rsi),%xmm0
    40002165:	f3 0f 6f 4e 10       	movdqu 0x10(%rsi),%xmm1
    4000216a:	f3 0f 6f 56 20       	movdqu 0x20(%rsi),%xmm2
    4000216f:	48 8b 46 30          	mov    0x30(%rsi),%rax
    40002173:	48 63 f7             	movslq %edi,%rsi
    40002176:	bf 2f 00 00 00       	mov    $0x2f,%edi
    4000217b:	0f 29 4c 24 18       	movaps %xmm1,0x18(%rsp)
    40002180:	0f 29 54 24 28       	movaps %xmm2,0x28(%rsp)
    40002185:	0f 29 44 24 08       	movaps %xmm0,0x8(%rsp)
    4000218a:	48 89 44 24 38       	mov    %rax,0x38(%rsp)
    4000218f:	c7 44 24 34 00 00 00 	movl   $0x0,0x34(%rsp)
    40002196:	00 
    40002197:	c7 44 24 24 00 00 00 	movl   $0x0,0x24(%rsp)
    4000219e:	00 
    4000219f:	6a 00                	push   $0x0
    400021a1:	48 8d 54 24 10       	lea    0x10(%rsp),%rdx
    400021a6:	e8 0f f4 ff ff       	call   400015ba <__syscall_cp>
    400021ab:	48 89 c7             	mov    %rax,%rdi
    400021ae:	e8 8d fd ff ff       	call   40001f40 <__syscall_ret>
    400021b3:	66 0f 6f 5c 24 10    	movdqa 0x10(%rsp),%xmm3
    400021b9:	66 0f 6f 64 24 20    	movdqa 0x20(%rsp),%xmm4
    400021bf:	66 0f 6f 6c 24 30    	movdqa 0x30(%rsp),%xmm5
    400021c5:	48 8b 54 24 40       	mov    0x40(%rsp),%rdx
    400021ca:	0f 11 1b             	movups %xmm3,(%rbx)
    400021cd:	48 89 53 30          	mov    %rdx,0x30(%rbx)
    400021d1:	0f 11 63 10          	movups %xmm4,0x10(%rbx)
    400021d5:	0f 11 6b 20          	movups %xmm5,0x20(%rbx)
    400021d9:	48 83 c4 50          	add    $0x50,%rsp
    400021dd:	5b                   	pop    %rbx
    400021de:	c3                   	ret    
    400021df:	90                   	nop

00000000400021e0 <sendmsg>:
    400021e0:	41 54                	push   %r12
    400021e2:	41 89 d4             	mov    %edx,%r12d
    400021e5:	55                   	push   %rbp
    400021e6:	89 fd                	mov    %edi,%ebp
    400021e8:	53                   	push   %rbx
    400021e9:	48 81 ec 60 04 00 00 	sub    $0x460,%rsp
    400021f0:	48 85 f6             	test   %rsi,%rsi
    400021f3:	0f 84 df 00 00 00    	je     400022d8 <sendmsg+0xf8>
    400021f9:	f3 0f 6f 56 20       	movdqu 0x20(%rsi),%xmm2
    400021fe:	f3 0f 6f 06          	movdqu (%rsi),%xmm0
    40002202:	f3 0f 6f 4e 10       	movdqu 0x10(%rsi),%xmm1
    40002207:	48 8b 46 30          	mov    0x30(%rsi),%rax
    4000220b:	0f 29 54 24 20       	movaps %xmm2,0x20(%rsp)
    40002210:	8b 54 24 28          	mov    0x28(%rsp),%edx
    40002214:	0f 29 4c 24 10       	movaps %xmm1,0x10(%rsp)
    40002219:	48 89 44 24 30       	mov    %rax,0x30(%rsp)
    4000221e:	c7 44 24 2c 00 00 00 	movl   $0x0,0x2c(%rsp)
    40002225:	00 
    40002226:	c7 44 24 1c 00 00 00 	movl   $0x0,0x1c(%rsp)
    4000222d:	00 
    4000222e:	0f 29 04 24          	movaps %xmm0,(%rsp)
    40002232:	85 d2                	test   %edx,%edx
    40002234:	74 6a                	je     400022a0 <sendmsg+0xc0>
    40002236:	81 fa 20 04 00 00    	cmp    $0x420,%edx
    4000223c:	0f 87 9a 00 00 00    	ja     400022dc <sendmsg+0xfc>
    40002242:	48 8d 5c 24 40       	lea    0x40(%rsp),%rbx
    40002247:	48 8b 74 24 20       	mov    0x20(%rsp),%rsi
    4000224c:	48 89 df             	mov    %rbx,%rdi
    4000224f:	e8 59 27 00 00       	call   400049ad <memcpy>
    40002254:	8b 4c 24 28          	mov    0x28(%rsp),%ecx
    40002258:	48 89 5c 24 20       	mov    %rbx,0x20(%rsp)
    4000225d:	83 f9 0f             	cmp    $0xf,%ecx
    40002260:	76 3e                	jbe    400022a0 <sendmsg+0xc0>
    40002262:	48 89 da             	mov    %rbx,%rdx
    40002265:	48 01 d9             	add    %rbx,%rcx
    40002268:	eb 20                	jmp    4000228a <sendmsg+0xaa>
    4000226a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40002270:	48 83 c0 07          	add    $0x7,%rax
    40002274:	48 89 ce             	mov    %rcx,%rsi
    40002277:	48 83 e0 f8          	and    $0xfffffffffffffff8,%rax
    4000227b:	48 29 d6             	sub    %rdx,%rsi
    4000227e:	48 8d 78 10          	lea    0x10(%rax),%rdi
    40002282:	48 39 f7             	cmp    %rsi,%rdi
    40002285:	73 19                	jae    400022a0 <sendmsg+0xc0>
    40002287:	48 01 c2             	add    %rax,%rdx
    4000228a:	8b 02                	mov    (%rdx),%eax
    4000228c:	c7 42 04 00 00 00 00 	movl   $0x0,0x4(%rdx)
    40002293:	83 f8 0f             	cmp    $0xf,%eax
    40002296:	77 d8                	ja     40002270 <sendmsg+0x90>
    40002298:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4000229f:	00 
    400022a0:	48 89 e2             	mov    %rsp,%rdx
    400022a3:	48 83 ec 08          	sub    $0x8,%rsp
    400022a7:	49 63 cc             	movslq %r12d,%rcx
    400022aa:	48 63 f5             	movslq %ebp,%rsi
    400022ad:	45 31 c9             	xor    %r9d,%r9d
    400022b0:	6a 00                	push   $0x0
    400022b2:	45 31 c0             	xor    %r8d,%r8d
    400022b5:	bf 2e 00 00 00       	mov    $0x2e,%edi
    400022ba:	e8 fb f2 ff ff       	call   400015ba <__syscall_cp>
    400022bf:	48 89 c7             	mov    %rax,%rdi
    400022c2:	e8 79 fc ff ff       	call   40001f40 <__syscall_ret>
    400022c7:	5a                   	pop    %rdx
    400022c8:	59                   	pop    %rcx
    400022c9:	48 81 c4 60 04 00 00 	add    $0x460,%rsp
    400022d0:	5b                   	pop    %rbx
    400022d1:	5d                   	pop    %rbp
    400022d2:	41 5c                	pop    %r12
    400022d4:	c3                   	ret    
    400022d5:	0f 1f 00             	nopl   (%rax)
    400022d8:	31 d2                	xor    %edx,%edx
    400022da:	eb c7                	jmp    400022a3 <sendmsg+0xc3>
    400022dc:	e8 5f 2d 00 00       	call   40005040 <__errno_location>
    400022e1:	c7 00 0c 00 00 00    	movl   $0xc,(%rax)
    400022e7:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    400022ee:	eb d9                	jmp    400022c9 <sendmsg+0xe9>

00000000400022f0 <socketpair>:
    400022f0:	41 56                	push   %r14
    400022f2:	48 63 f6             	movslq %esi,%rsi
    400022f5:	49 89 ca             	mov    %rcx,%r10
    400022f8:	45 31 c0             	xor    %r8d,%r8d
    400022fb:	41 55                	push   %r13
    400022fd:	4c 63 ea             	movslq %edx,%r13
    40002300:	45 31 c9             	xor    %r9d,%r9d
    40002303:	b8 35 00 00 00       	mov    $0x35,%eax
    40002308:	41 54                	push   %r12
    4000230a:	4c 63 e7             	movslq %edi,%r12
    4000230d:	4c 89 ea             	mov    %r13,%rdx
    40002310:	55                   	push   %rbp
    40002311:	4c 89 e7             	mov    %r12,%rdi
    40002314:	48 89 cd             	mov    %rcx,%rbp
    40002317:	53                   	push   %rbx
    40002318:	48 89 f3             	mov    %rsi,%rbx
    4000231b:	0f 05                	syscall 
    4000231d:	48 89 c7             	mov    %rax,%rdi
    40002320:	e8 1b fc ff ff       	call   40001f40 <__syscall_ret>
    40002325:	41 89 c6             	mov    %eax,%r14d
    40002328:	85 c0                	test   %eax,%eax
    4000232a:	78 14                	js     40002340 <socketpair+0x50>
    4000232c:	5b                   	pop    %rbx
    4000232d:	44 89 f0             	mov    %r14d,%eax
    40002330:	5d                   	pop    %rbp
    40002331:	41 5c                	pop    %r12
    40002333:	41 5d                	pop    %r13
    40002335:	41 5e                	pop    %r14
    40002337:	c3                   	ret    
    40002338:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4000233f:	00 
    40002340:	e8 fb 2c 00 00       	call   40005040 <__errno_location>
    40002345:	8b 00                	mov    (%rax),%eax
    40002347:	83 f8 16             	cmp    $0x16,%eax
    4000234a:	74 05                	je     40002351 <socketpair+0x61>
    4000234c:	83 f8 5d             	cmp    $0x5d,%eax
    4000234f:	75 db                	jne    4000232c <socketpair+0x3c>
    40002351:	f7 c3 00 08 08 00    	test   $0x80800,%ebx
    40002357:	74 d3                	je     4000232c <socketpair+0x3c>
    40002359:	89 de                	mov    %ebx,%esi
    4000235b:	49 89 ea             	mov    %rbp,%r10
    4000235e:	45 31 c0             	xor    %r8d,%r8d
    40002361:	45 31 c9             	xor    %r9d,%r9d
    40002364:	81 e6 ff f7 f7 ff    	and    $0xfff7f7ff,%esi
    4000236a:	b8 35 00 00 00       	mov    $0x35,%eax
    4000236f:	4c 89 e7             	mov    %r12,%rdi
    40002372:	4c 89 ea             	mov    %r13,%rdx
    40002375:	48 63 f6             	movslq %esi,%rsi
    40002378:	0f 05                	syscall 
    4000237a:	48 89 c7             	mov    %rax,%rdi
    4000237d:	e8 be fb ff ff       	call   40001f40 <__syscall_ret>
    40002382:	41 89 c6             	mov    %eax,%r14d
    40002385:	85 c0                	test   %eax,%eax
    40002387:	78 a3                	js     4000232c <socketpair+0x3c>
    40002389:	f7 c3 00 00 08 00    	test   $0x80000,%ebx
    4000238f:	74 22                	je     400023b3 <socketpair+0xc3>
    40002391:	41 b8 48 00 00 00    	mov    $0x48,%r8d
    40002397:	48 63 7d 00          	movslq 0x0(%rbp),%rdi
    4000239b:	be 02 00 00 00       	mov    $0x2,%esi
    400023a0:	ba 01 00 00 00       	mov    $0x1,%edx
    400023a5:	4c 89 c0             	mov    %r8,%rax
    400023a8:	0f 05                	syscall 
    400023aa:	48 63 7d 04          	movslq 0x4(%rbp),%rdi
    400023ae:	4c 89 c0             	mov    %r8,%rax
    400023b1:	0f 05                	syscall 
    400023b3:	80 e7 08             	and    $0x8,%bh
    400023b6:	0f 84 70 ff ff ff    	je     4000232c <socketpair+0x3c>
    400023bc:	41 b8 48 00 00 00    	mov    $0x48,%r8d
    400023c2:	48 63 7d 00          	movslq 0x0(%rbp),%rdi
    400023c6:	be 04 00 00 00       	mov    $0x4,%esi
    400023cb:	ba 00 08 00 00       	mov    $0x800,%edx
    400023d0:	4c 89 c0             	mov    %r8,%rax
    400023d3:	0f 05                	syscall 
    400023d5:	48 63 7d 04          	movslq 0x4(%rbp),%rdi
    400023d9:	4c 89 c0             	mov    %r8,%rax
    400023dc:	0f 05                	syscall 
    400023de:	e9 49 ff ff ff       	jmp    4000232c <socketpair+0x3c>
    400023e3:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    400023ea:	00 00 00 
    400023ed:	0f 1f 00             	nopl   (%rax)

00000000400023f0 <poll>:
    400023f0:	48 83 ec 10          	sub    $0x10,%rsp
    400023f4:	48 63 ca             	movslq %edx,%rcx
    400023f7:	45 31 c9             	xor    %r9d,%r9d
    400023fa:	48 89 f2             	mov    %rsi,%rdx
    400023fd:	6a 00                	push   $0x0
    400023ff:	48 89 fe             	mov    %rdi,%rsi
    40002402:	45 31 c0             	xor    %r8d,%r8d
    40002405:	bf 07 00 00 00       	mov    $0x7,%edi
    4000240a:	e8 ab f1 ff ff       	call   400015ba <__syscall_cp>
    4000240f:	48 89 c7             	mov    %rax,%rdi
    40002412:	e8 29 fb ff ff       	call   40001f40 <__syscall_ret>
    40002417:	48 83 c4 18          	add    $0x18,%rsp
    4000241b:	c3                   	ret    
    4000241c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000040002420 <stat>:
    40002420:	48 89 f2             	mov    %rsi,%rdx
    40002423:	31 c9                	xor    %ecx,%ecx
    40002425:	48 89 fe             	mov    %rdi,%rsi
    40002428:	bf 9c ff ff ff       	mov    $0xffffff9c,%edi
    4000242d:	e9 ce 30 00 00       	jmp    40005500 <__fstatat>
    40002432:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40002439:	00 00 00 
    4000243c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000040002440 <printf>:
    40002440:	48 81 ec d8 00 00 00 	sub    $0xd8,%rsp
    40002447:	49 89 fa             	mov    %rdi,%r10
    4000244a:	48 89 74 24 28       	mov    %rsi,0x28(%rsp)
    4000244f:	48 89 54 24 30       	mov    %rdx,0x30(%rsp)
    40002454:	48 89 4c 24 38       	mov    %rcx,0x38(%rsp)
    40002459:	4c 89 44 24 40       	mov    %r8,0x40(%rsp)
    4000245e:	4c 89 4c 24 48       	mov    %r9,0x48(%rsp)
    40002463:	84 c0                	test   %al,%al
    40002465:	74 37                	je     4000249e <printf+0x5e>
    40002467:	0f 29 44 24 50       	movaps %xmm0,0x50(%rsp)
    4000246c:	0f 29 4c 24 60       	movaps %xmm1,0x60(%rsp)
    40002471:	0f 29 54 24 70       	movaps %xmm2,0x70(%rsp)
    40002476:	0f 29 9c 24 80 00 00 	movaps %xmm3,0x80(%rsp)
    4000247d:	00 
    4000247e:	0f 29 a4 24 90 00 00 	movaps %xmm4,0x90(%rsp)
    40002485:	00 
    40002486:	0f 29 ac 24 a0 00 00 	movaps %xmm5,0xa0(%rsp)
    4000248d:	00 
    4000248e:	0f 29 b4 24 b0 00 00 	movaps %xmm6,0xb0(%rsp)
    40002495:	00 
    40002496:	0f 29 bc 24 c0 00 00 	movaps %xmm7,0xc0(%rsp)
    4000249d:	00 
    4000249e:	48 8d 84 24 e0 00 00 	lea    0xe0(%rsp),%rax
    400024a5:	00 
    400024a6:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
    400024ab:	4c 89 d6             	mov    %r10,%rsi
    400024ae:	bf 80 7c 00 40       	mov    $0x40007c80,%edi
    400024b3:	48 89 44 24 10       	mov    %rax,0x10(%rsp)
    400024b8:	48 8d 44 24 20       	lea    0x20(%rsp),%rax
    400024bd:	c7 44 24 08 08 00 00 	movl   $0x8,0x8(%rsp)
    400024c4:	00 
    400024c5:	c7 44 24 0c 30 00 00 	movl   $0x30,0xc(%rsp)
    400024cc:	00 
    400024cd:	48 89 44 24 18       	mov    %rax,0x18(%rsp)
    400024d2:	e8 f9 22 00 00       	call   400047d0 <vfprintf>
    400024d7:	48 81 c4 d8 00 00 00 	add    $0xd8,%rsp
    400024de:	c3                   	ret    
    400024df:	90                   	nop

00000000400024e0 <pop_arg>:
    400024e0:	83 ee 09             	sub    $0x9,%esi
    400024e3:	48 89 d0             	mov    %rdx,%rax
    400024e6:	83 fe 11             	cmp    $0x11,%esi
    400024e9:	77 45                	ja     40002530 <pop_arg+0x50>
    400024eb:	ff 24 f5 b8 6b 00 40 	jmp    *0x40006bb8(,%rsi,8)
    400024f2:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    400024f8:	8b 12                	mov    (%rdx),%edx
    400024fa:	83 fa 2f             	cmp    $0x2f,%edx
    400024fd:	77 39                	ja     40002538 <pop_arg+0x58>
    400024ff:	89 d1                	mov    %edx,%ecx
    40002501:	83 c2 08             	add    $0x8,%edx
    40002504:	48 03 48 10          	add    0x10(%rax),%rcx
    40002508:	89 10                	mov    %edx,(%rax)
    4000250a:	48 8b 01             	mov    (%rcx),%rax
    4000250d:	48 89 07             	mov    %rax,(%rdi)
    40002510:	c3                   	ret    
    40002511:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40002518:	48 8b 4a 08          	mov    0x8(%rdx),%rcx
    4000251c:	48 8d 51 0f          	lea    0xf(%rcx),%rdx
    40002520:	48 83 e2 f0          	and    $0xfffffffffffffff0,%rdx
    40002524:	48 8d 4a 10          	lea    0x10(%rdx),%rcx
    40002528:	48 89 48 08          	mov    %rcx,0x8(%rax)
    4000252c:	db 2a                	fldt   (%rdx)
    4000252e:	db 3f                	fstpt  (%rdi)
    40002530:	c3                   	ret    
    40002531:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40002538:	48 8b 48 08          	mov    0x8(%rax),%rcx
    4000253c:	48 8d 51 08          	lea    0x8(%rcx),%rdx
    40002540:	48 89 50 08          	mov    %rdx,0x8(%rax)
    40002544:	48 8b 01             	mov    (%rcx),%rax
    40002547:	48 89 07             	mov    %rax,(%rdi)
    4000254a:	c3                   	ret    
    4000254b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40002550:	8b 12                	mov    (%rdx),%edx
    40002552:	83 fa 2f             	cmp    $0x2f,%edx
    40002555:	0f 87 4d 01 00 00    	ja     400026a8 <pop_arg+0x1c8>
    4000255b:	89 d1                	mov    %edx,%ecx
    4000255d:	83 c2 08             	add    $0x8,%edx
    40002560:	48 03 48 10          	add    0x10(%rax),%rcx
    40002564:	89 10                	mov    %edx,(%rax)
    40002566:	0f b7 01             	movzwl (%rcx),%eax
    40002569:	48 89 07             	mov    %rax,(%rdi)
    4000256c:	c3                   	ret    
    4000256d:	0f 1f 00             	nopl   (%rax)
    40002570:	8b 52 04             	mov    0x4(%rdx),%edx
    40002573:	81 fa af 00 00 00    	cmp    $0xaf,%edx
    40002579:	0f 87 11 01 00 00    	ja     40002690 <pop_arg+0x1b0>
    4000257f:	89 d1                	mov    %edx,%ecx
    40002581:	83 c2 10             	add    $0x10,%edx
    40002584:	48 03 48 10          	add    0x10(%rax),%rcx
    40002588:	89 50 04             	mov    %edx,0x4(%rax)
    4000258b:	dd 01                	fldl   (%rcx)
    4000258d:	db 3f                	fstpt  (%rdi)
    4000258f:	c3                   	ret    
    40002590:	8b 12                	mov    (%rdx),%edx
    40002592:	83 fa 2f             	cmp    $0x2f,%edx
    40002595:	0f 87 dd 00 00 00    	ja     40002678 <pop_arg+0x198>
    4000259b:	89 d1                	mov    %edx,%ecx
    4000259d:	83 c2 08             	add    $0x8,%edx
    400025a0:	48 03 48 10          	add    0x10(%rax),%rcx
    400025a4:	89 10                	mov    %edx,(%rax)
    400025a6:	48 63 01             	movslq (%rcx),%rax
    400025a9:	48 89 07             	mov    %rax,(%rdi)
    400025ac:	c3                   	ret    
    400025ad:	0f 1f 00             	nopl   (%rax)
    400025b0:	8b 12                	mov    (%rdx),%edx
    400025b2:	83 fa 2f             	cmp    $0x2f,%edx
    400025b5:	0f 87 a5 00 00 00    	ja     40002660 <pop_arg+0x180>
    400025bb:	89 d1                	mov    %edx,%ecx
    400025bd:	83 c2 08             	add    $0x8,%edx
    400025c0:	48 03 48 10          	add    0x10(%rax),%rcx
    400025c4:	89 10                	mov    %edx,(%rax)
    400025c6:	8b 01                	mov    (%rcx),%eax
    400025c8:	48 89 07             	mov    %rax,(%rdi)
    400025cb:	c3                   	ret    
    400025cc:	0f 1f 40 00          	nopl   0x0(%rax)
    400025d0:	8b 12                	mov    (%rdx),%edx
    400025d2:	83 fa 2f             	cmp    $0x2f,%edx
    400025d5:	77 79                	ja     40002650 <pop_arg+0x170>
    400025d7:	89 d1                	mov    %edx,%ecx
    400025d9:	83 c2 08             	add    $0x8,%edx
    400025dc:	48 03 48 10          	add    0x10(%rax),%rcx
    400025e0:	89 10                	mov    %edx,(%rax)
    400025e2:	0f b6 01             	movzbl (%rcx),%eax
    400025e5:	48 89 07             	mov    %rax,(%rdi)
    400025e8:	c3                   	ret    
    400025e9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    400025f0:	8b 12                	mov    (%rdx),%edx
    400025f2:	83 fa 2f             	cmp    $0x2f,%edx
    400025f5:	77 49                	ja     40002640 <pop_arg+0x160>
    400025f7:	89 d1                	mov    %edx,%ecx
    400025f9:	83 c2 08             	add    $0x8,%edx
    400025fc:	48 03 48 10          	add    0x10(%rax),%rcx
    40002600:	89 10                	mov    %edx,(%rax)
    40002602:	48 0f be 01          	movsbq (%rcx),%rax
    40002606:	48 89 07             	mov    %rax,(%rdi)
    40002609:	c3                   	ret    
    4000260a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40002610:	8b 12                	mov    (%rdx),%edx
    40002612:	83 fa 2f             	cmp    $0x2f,%edx
    40002615:	77 19                	ja     40002630 <pop_arg+0x150>
    40002617:	89 d1                	mov    %edx,%ecx
    40002619:	83 c2 08             	add    $0x8,%edx
    4000261c:	48 03 48 10          	add    0x10(%rax),%rcx
    40002620:	89 10                	mov    %edx,(%rax)
    40002622:	48 0f bf 01          	movswq (%rcx),%rax
    40002626:	48 89 07             	mov    %rax,(%rdi)
    40002629:	c3                   	ret    
    4000262a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40002630:	48 8b 48 08          	mov    0x8(%rax),%rcx
    40002634:	48 8d 51 08          	lea    0x8(%rcx),%rdx
    40002638:	48 89 50 08          	mov    %rdx,0x8(%rax)
    4000263c:	eb e4                	jmp    40002622 <pop_arg+0x142>
    4000263e:	66 90                	xchg   %ax,%ax
    40002640:	48 8b 48 08          	mov    0x8(%rax),%rcx
    40002644:	48 8d 51 08          	lea    0x8(%rcx),%rdx
    40002648:	48 89 50 08          	mov    %rdx,0x8(%rax)
    4000264c:	eb b4                	jmp    40002602 <pop_arg+0x122>
    4000264e:	66 90                	xchg   %ax,%ax
    40002650:	48 8b 48 08          	mov    0x8(%rax),%rcx
    40002654:	48 8d 51 08          	lea    0x8(%rcx),%rdx
    40002658:	48 89 50 08          	mov    %rdx,0x8(%rax)
    4000265c:	eb 84                	jmp    400025e2 <pop_arg+0x102>
    4000265e:	66 90                	xchg   %ax,%ax
    40002660:	48 8b 48 08          	mov    0x8(%rax),%rcx
    40002664:	48 8d 51 08          	lea    0x8(%rcx),%rdx
    40002668:	48 89 50 08          	mov    %rdx,0x8(%rax)
    4000266c:	e9 55 ff ff ff       	jmp    400025c6 <pop_arg+0xe6>
    40002671:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40002678:	48 8b 48 08          	mov    0x8(%rax),%rcx
    4000267c:	48 8d 51 08          	lea    0x8(%rcx),%rdx
    40002680:	48 89 50 08          	mov    %rdx,0x8(%rax)
    40002684:	e9 1d ff ff ff       	jmp    400025a6 <pop_arg+0xc6>
    40002689:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40002690:	48 8b 48 08          	mov    0x8(%rax),%rcx
    40002694:	48 8d 51 08          	lea    0x8(%rcx),%rdx
    40002698:	48 89 50 08          	mov    %rdx,0x8(%rax)
    4000269c:	e9 ea fe ff ff       	jmp    4000258b <pop_arg+0xab>
    400026a1:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    400026a8:	48 8b 48 08          	mov    0x8(%rax),%rcx
    400026ac:	48 8d 51 08          	lea    0x8(%rcx),%rdx
    400026b0:	48 89 50 08          	mov    %rdx,0x8(%rax)
    400026b4:	e9 ad fe ff ff       	jmp    40002566 <pop_arg+0x86>
    400026b9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

00000000400026c0 <pad.part.0>:
    400026c0:	41 54                	push   %r12
    400026c2:	29 ca                	sub    %ecx,%edx
    400026c4:	49 89 fc             	mov    %rdi,%r12
    400026c7:	40 0f be f6          	movsbl %sil,%esi
    400026cb:	55                   	push   %rbp
    400026cc:	48 63 ea             	movslq %edx,%rbp
    400026cf:	ba 00 01 00 00       	mov    $0x100,%edx
    400026d4:	53                   	push   %rbx
    400026d5:	48 81 ec 00 01 00 00 	sub    $0x100,%rsp
    400026dc:	39 d5                	cmp    %edx,%ebp
    400026de:	48 0f 4e d5          	cmovle %rbp,%rdx
    400026e2:	48 89 e7             	mov    %rsp,%rdi
    400026e5:	e8 f5 22 00 00       	call   400049df <memset>
    400026ea:	81 fd ff 00 00 00    	cmp    $0xff,%ebp
    400026f0:	7e 7e                	jle    40002770 <pad.part.0+0xb0>
    400026f2:	41 8b 04 24          	mov    (%r12),%eax
    400026f6:	89 eb                	mov    %ebp,%ebx
    400026f8:	eb 16                	jmp    40002710 <pad.part.0+0x50>
    400026fa:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40002700:	81 eb 00 01 00 00    	sub    $0x100,%ebx
    40002706:	89 dd                	mov    %ebx,%ebp
    40002708:	81 fb ff 00 00 00    	cmp    $0xff,%ebx
    4000270e:	76 30                	jbe    40002740 <pad.part.0+0x80>
    40002710:	89 c2                	mov    %eax,%edx
    40002712:	83 e2 20             	and    $0x20,%edx
    40002715:	75 e9                	jne    40002700 <pad.part.0+0x40>
    40002717:	4c 89 e2             	mov    %r12,%rdx
    4000271a:	be 00 01 00 00       	mov    $0x100,%esi
    4000271f:	48 89 e7             	mov    %rsp,%rdi
    40002722:	81 eb 00 01 00 00    	sub    $0x100,%ebx
    40002728:	e8 03 32 00 00       	call   40005930 <__fwritex>
    4000272d:	41 8b 04 24          	mov    (%r12),%eax
    40002731:	89 dd                	mov    %ebx,%ebp
    40002733:	89 c2                	mov    %eax,%edx
    40002735:	83 e2 20             	and    $0x20,%edx
    40002738:	81 fb ff 00 00 00    	cmp    $0xff,%ebx
    4000273e:	77 d0                	ja     40002710 <pad.part.0+0x50>
    40002740:	85 d2                	test   %edx,%edx
    40002742:	74 0c                	je     40002750 <pad.part.0+0x90>
    40002744:	48 81 c4 00 01 00 00 	add    $0x100,%rsp
    4000274b:	5b                   	pop    %rbx
    4000274c:	5d                   	pop    %rbp
    4000274d:	41 5c                	pop    %r12
    4000274f:	c3                   	ret    
    40002750:	48 63 f5             	movslq %ebp,%rsi
    40002753:	4c 89 e2             	mov    %r12,%rdx
    40002756:	48 89 e7             	mov    %rsp,%rdi
    40002759:	e8 d2 31 00 00       	call   40005930 <__fwritex>
    4000275e:	48 81 c4 00 01 00 00 	add    $0x100,%rsp
    40002765:	5b                   	pop    %rbx
    40002766:	5d                   	pop    %rbp
    40002767:	41 5c                	pop    %r12
    40002769:	c3                   	ret    
    4000276a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40002770:	41 8b 14 24          	mov    (%r12),%edx
    40002774:	83 e2 20             	and    $0x20,%edx
    40002777:	eb c7                	jmp    40002740 <pad.part.0+0x80>
    40002779:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000040002780 <fmt_fp>:
    40002780:	41 57                	push   %r15
    40002782:	41 56                	push   %r14
    40002784:	41 55                	push   %r13
    40002786:	49 89 fd             	mov    %rdi,%r13
    40002789:	41 54                	push   %r12
    4000278b:	41 89 d4             	mov    %edx,%r12d
    4000278e:	55                   	push   %rbp
    4000278f:	53                   	push   %rbx
    40002790:	48 81 ec 38 1d 00 00 	sub    $0x1d38,%rsp
    40002797:	89 74 24 38          	mov    %esi,0x38(%rsp)
    4000279b:	89 4c 24 14          	mov    %ecx,0x14(%rsp)
    4000279f:	44 89 44 24 18       	mov    %r8d,0x18(%rsp)
    400027a4:	c7 44 24 50 00 00 00 	movl   $0x0,0x50(%rsp)
    400027ab:	00 
    400027ac:	ff b4 24 78 1d 00 00 	push   0x1d78(%rsp)
    400027b3:	ff b4 24 78 1d 00 00 	push   0x1d78(%rsp)
    400027ba:	e8 71 2c 00 00       	call   40005430 <__signbitl>
    400027bf:	41 5b                	pop    %r11
    400027c1:	5b                   	pop    %rbx
    400027c2:	85 c0                	test   %eax,%eax
    400027c4:	0f 84 76 03 00 00    	je     40002b40 <fmt_fp+0x3c0>
    400027ca:	db ac 24 70 1d 00 00 	fldt   0x1d70(%rsp)
    400027d1:	c7 44 24 20 01 00 00 	movl   $0x1,0x20(%rsp)
    400027d8:	00 
    400027d9:	48 c7 44 24 30 48 6c 	movq   $0x40006c48,0x30(%rsp)
    400027e0:	00 40 
    400027e2:	d9 e0                	fchs   
    400027e4:	db bc 24 70 1d 00 00 	fstpt  0x1d70(%rsp)
    400027eb:	ff b4 24 78 1d 00 00 	push   0x1d78(%rsp)
    400027f2:	ff b4 24 78 1d 00 00 	push   0x1d78(%rsp)
    400027f9:	e8 e2 2b 00 00       	call   400053e0 <__fpclassifyl>
    400027fe:	41 59                	pop    %r9
    40002800:	41 5a                	pop    %r10
    40002802:	83 f8 01             	cmp    $0x1,%eax
    40002805:	0f 8e 4d 06 00 00    	jle    40002e58 <fmt_fp+0x6d8>
    4000280b:	ff b4 24 78 1d 00 00 	push   0x1d78(%rsp)
    40002812:	ff b4 24 78 1d 00 00 	push   0x1d78(%rsp)
    40002819:	48 8d 7c 24 60       	lea    0x60(%rsp),%rdi
    4000281e:	e8 1d 2c 00 00       	call   40005440 <frexpl>
    40002823:	5f                   	pop    %rdi
    40002824:	41 58                	pop    %r8
    40002826:	d8 c0                	fadd   %st(0),%st
    40002828:	d9 ee                	fldz   
    4000282a:	d9 c0                	fld    %st(0)
    4000282c:	d9 ca                	fxch   %st(2)
    4000282e:	db ea                	fucomi %st(2),%st
    40002830:	dd da                	fstp   %st(2)
    40002832:	0f 8a 30 03 00 00    	jp     40002b68 <fmt_fp+0x3e8>
    40002838:	0f 85 2a 03 00 00    	jne    40002b68 <fmt_fp+0x3e8>
    4000283e:	44 8b 7c 24 18       	mov    0x18(%rsp),%r15d
    40002843:	44 8b 4c 24 50       	mov    0x50(%rsp),%r9d
    40002848:	41 83 cf 20          	or     $0x20,%r15d
    4000284c:	41 83 ff 61          	cmp    $0x61,%r15d
    40002850:	0f 84 32 03 00 00    	je     40002b88 <fmt_fp+0x408>
    40002856:	45 85 e4             	test   %r12d,%r12d
    40002859:	0f 88 78 0b 00 00    	js     400033d7 <fmt_fp+0xc57>
    4000285f:	0f 95 44 24 3c       	setne  0x3c(%rsp)
    40002864:	d9 c0                	fld    %st(0)
    40002866:	d9 ca                	fxch   %st(2)
    40002868:	db ea                	fucomi %st(2),%st
    4000286a:	dd da                	fstp   %st(2)
    4000286c:	0f 8a d7 06 00 00    	jp     40002f49 <fmt_fp+0x7c9>
    40002872:	0f 85 d8 06 00 00    	jne    40002f50 <fmt_fp+0x7d0>
    40002878:	d9 7c 24 4e          	fnstcw 0x4e(%rsp)
    4000287c:	45 85 c9             	test   %r9d,%r9d
    4000287f:	48 8d 84 24 80 00 00 	lea    0x80(%rsp),%rax
    40002886:	00 
    40002887:	48 8d 94 24 28 1c 00 	lea    0x1c28(%rsp),%rdx
    4000288e:	00 
    4000288f:	48 0f 49 c2          	cmovns %rdx,%rax
    40002893:	d9 05 ef 47 00 00    	flds   0x47ef(%rip)        # 40007088 <states+0x1e8>
    40002899:	d9 ca                	fxch   %st(2)
    4000289b:	48 89 44 24 28       	mov    %rax,0x28(%rsp)
    400028a0:	48 89 c3             	mov    %rax,%rbx
    400028a3:	0f b7 44 24 4e       	movzwl 0x4e(%rsp),%eax
    400028a8:	80 cc 0c             	or     $0xc,%ah
    400028ab:	66 89 44 24 4c       	mov    %ax,0x4c(%rsp)
    400028b0:	d9 c0                	fld    %st(0)
    400028b2:	d9 6c 24 4c          	fldcw  0x4c(%rsp)
    400028b6:	df 7c 24 08          	fistpll 0x8(%rsp)
    400028ba:	d9 6c 24 4e          	fldcw  0x4e(%rsp)
    400028be:	48 83 c3 04          	add    $0x4,%rbx
    400028c2:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
    400028c7:	89 43 fc             	mov    %eax,-0x4(%rbx)
    400028ca:	89 c0                	mov    %eax,%eax
    400028cc:	48 89 44 24 08       	mov    %rax,0x8(%rsp)
    400028d1:	df 6c 24 08          	fildll 0x8(%rsp)
    400028d5:	de e9                	fsubrp %st,%st(1)
    400028d7:	d8 ca                	fmul   %st(2),%st
    400028d9:	d9 c1                	fld    %st(1)
    400028db:	d9 c9                	fxch   %st(1)
    400028dd:	db e9                	fucomi %st(1),%st
    400028df:	dd d9                	fstp   %st(1)
    400028e1:	7a cd                	jp     400028b0 <fmt_fp+0x130>
    400028e3:	75 cb                	jne    400028b0 <fmt_fp+0x130>
    400028e5:	dd d8                	fstp   %st(0)
    400028e7:	dd d8                	fstp   %st(0)
    400028e9:	dd d8                	fstp   %st(0)
    400028eb:	48 8b 6c 24 28       	mov    0x28(%rsp),%rbp
    400028f0:	45 85 c9             	test   %r9d,%r9d
    400028f3:	0f 8e 8c 00 00 00    	jle    40002985 <fmt_fp+0x205>
    400028f9:	49 b8 53 5a 9b a0 2f 	movabs $0x44b82fa09b5a53,%r8
    40002900:	b8 44 00 
    40002903:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40002908:	b9 1d 00 00 00       	mov    $0x1d,%ecx
    4000290d:	48 8d 7b fc          	lea    -0x4(%rbx),%rdi
    40002911:	41 39 c9             	cmp    %ecx,%r9d
    40002914:	41 0f 4e c9          	cmovle %r9d,%ecx
    40002918:	48 39 ef             	cmp    %rbp,%rdi
    4000291b:	72 3f                	jb     4000295c <fmt_fp+0x1dc>
    4000291d:	31 c0                	xor    %eax,%eax
    4000291f:	90                   	nop
    40002920:	8b 37                	mov    (%rdi),%esi
    40002922:	48 83 ef 04          	sub    $0x4,%rdi
    40002926:	48 d3 e6             	shl    %cl,%rsi
    40002929:	48 01 c6             	add    %rax,%rsi
    4000292c:	48 89 f2             	mov    %rsi,%rdx
    4000292f:	48 c1 ea 09          	shr    $0x9,%rdx
    40002933:	48 89 d0             	mov    %rdx,%rax
    40002936:	49 f7 e0             	mul    %r8
    40002939:	48 c1 ea 0b          	shr    $0xb,%rdx
    4000293d:	48 69 c2 00 ca 9a 3b 	imul   $0x3b9aca00,%rdx,%rax
    40002944:	48 29 c6             	sub    %rax,%rsi
    40002947:	89 d0                	mov    %edx,%eax
    40002949:	89 77 04             	mov    %esi,0x4(%rdi)
    4000294c:	48 39 ef             	cmp    %rbp,%rdi
    4000294f:	73 cf                	jae    40002920 <fmt_fp+0x1a0>
    40002951:	85 c0                	test   %eax,%eax
    40002953:	74 07                	je     4000295c <fmt_fp+0x1dc>
    40002955:	89 55 fc             	mov    %edx,-0x4(%rbp)
    40002958:	48 83 ed 04          	sub    $0x4,%rbp
    4000295c:	48 39 dd             	cmp    %rbx,%rbp
    4000295f:	72 10                	jb     40002971 <fmt_fp+0x1f1>
    40002961:	eb 15                	jmp    40002978 <fmt_fp+0x1f8>
    40002963:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40002968:	48 83 eb 04          	sub    $0x4,%rbx
    4000296c:	48 39 eb             	cmp    %rbp,%rbx
    4000296f:	76 07                	jbe    40002978 <fmt_fp+0x1f8>
    40002971:	8b 73 fc             	mov    -0x4(%rbx),%esi
    40002974:	85 f6                	test   %esi,%esi
    40002976:	74 f0                	je     40002968 <fmt_fp+0x1e8>
    40002978:	41 29 c9             	sub    %ecx,%r9d
    4000297b:	45 85 c9             	test   %r9d,%r9d
    4000297e:	7f 88                	jg     40002908 <fmt_fp+0x188>
    40002980:	44 89 4c 24 50       	mov    %r9d,0x50(%rsp)
    40002985:	45 85 c9             	test   %r9d,%r9d
    40002988:	0f 84 d3 00 00 00    	je     40002a61 <fmt_fp+0x2e1>
    4000298e:	41 8d 44 24 1d       	lea    0x1d(%r12),%eax
    40002993:	4c 89 6c 24 08       	mov    %r13,0x8(%rsp)
    40002998:	4c 8b 6c 24 28       	mov    0x28(%rsp),%r13
    4000299d:	48 69 c0 39 8e e3 38 	imul   $0x38e38e39,%rax,%rax
    400029a4:	48 c1 e8 21          	shr    $0x21,%rax
    400029a8:	4c 8d 58 01          	lea    0x1(%rax),%r11
    400029ac:	4e 8d 34 9d 00 00 00 	lea    0x0(,%r11,4),%r14
    400029b3:	00 
    400029b4:	0f 1f 40 00          	nopl   0x0(%rax)
    400029b8:	41 83 f9 f7          	cmp    $0xfffffff7,%r9d
    400029bc:	0f 8d 6e 04 00 00    	jge    40002e30 <fmt_fp+0x6b0>
    400029c2:	8b 45 00             	mov    0x0(%rbp),%eax
    400029c5:	41 83 c1 09          	add    $0x9,%r9d
    400029c9:	b9 09 00 00 00       	mov    $0x9,%ecx
    400029ce:	48 39 dd             	cmp    %rbx,%rbp
    400029d1:	0f 83 6d 04 00 00    	jae    40002e44 <fmt_fp+0x6c4>
    400029d7:	41 b8 01 00 00 00    	mov    $0x1,%r8d
    400029dd:	41 ba 00 ca 9a 3b    	mov    $0x3b9aca00,%r10d
    400029e3:	48 89 ee             	mov    %rbp,%rsi
    400029e6:	31 d2                	xor    %edx,%edx
    400029e8:	41 d3 e0             	shl    %cl,%r8d
    400029eb:	41 d3 fa             	sar    %cl,%r10d
    400029ee:	41 83 e8 01          	sub    $0x1,%r8d
    400029f2:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    400029f8:	8b 06                	mov    (%rsi),%eax
    400029fa:	48 83 c6 04          	add    $0x4,%rsi
    400029fe:	89 c7                	mov    %eax,%edi
    40002a00:	44 21 c0             	and    %r8d,%eax
    40002a03:	41 0f af c2          	imul   %r10d,%eax
    40002a07:	d3 ef                	shr    %cl,%edi
    40002a09:	01 fa                	add    %edi,%edx
    40002a0b:	89 56 fc             	mov    %edx,-0x4(%rsi)
    40002a0e:	89 c2                	mov    %eax,%edx
    40002a10:	48 39 de             	cmp    %rbx,%rsi
    40002a13:	72 e3                	jb     400029f8 <fmt_fp+0x278>
    40002a15:	8b 4d 00             	mov    0x0(%rbp),%ecx
    40002a18:	48 8d 45 04          	lea    0x4(%rbp),%rax
    40002a1c:	85 c9                	test   %ecx,%ecx
    40002a1e:	48 0f 44 e8          	cmove  %rax,%rbp
    40002a22:	85 d2                	test   %edx,%edx
    40002a24:	74 06                	je     40002a2c <fmt_fp+0x2ac>
    40002a26:	89 13                	mov    %edx,(%rbx)
    40002a28:	48 83 c3 04          	add    $0x4,%rbx
    40002a2c:	41 83 ff 66          	cmp    $0x66,%r15d
    40002a30:	4c 89 e8             	mov    %r13,%rax
    40002a33:	48 89 da             	mov    %rbx,%rdx
    40002a36:	48 0f 45 c5          	cmovne %rbp,%rax
    40002a3a:	48 29 c2             	sub    %rax,%rdx
    40002a3d:	4c 01 f0             	add    %r14,%rax
    40002a40:	48 c1 fa 02          	sar    $0x2,%rdx
    40002a44:	4c 39 da             	cmp    %r11,%rdx
    40002a47:	48 0f 4f d8          	cmovg  %rax,%rbx
    40002a4b:	45 85 c9             	test   %r9d,%r9d
    40002a4e:	0f 85 64 ff ff ff    	jne    400029b8 <fmt_fp+0x238>
    40002a54:	c7 44 24 50 00 00 00 	movl   $0x0,0x50(%rsp)
    40002a5b:	00 
    40002a5c:	4c 8b 6c 24 08       	mov    0x8(%rsp),%r13
    40002a61:	48 39 eb             	cmp    %rbp,%rbx
    40002a64:	0f 87 9f 04 00 00    	ja     40002f09 <fmt_fp+0x789>
    40002a6a:	44 89 e0             	mov    %r12d,%eax
    40002a6d:	31 ff                	xor    %edi,%edi
    40002a6f:	31 d2                	xor    %edx,%edx
    40002a71:	41 83 ff 67          	cmp    $0x67,%r15d
    40002a75:	48 8b 74 24 28       	mov    0x28(%rsp),%rsi
    40002a7a:	0f 94 c2             	sete   %dl
    40002a7d:	23 54 24 3c          	and    0x3c(%rsp),%edx
    40002a81:	29 d0                	sub    %edx,%eax
    40002a83:	48 89 da             	mov    %rbx,%rdx
    40002a86:	48 29 f2             	sub    %rsi,%rdx
    40002a89:	48 63 c8             	movslq %eax,%rcx
    40002a8c:	48 c1 fa 02          	sar    $0x2,%rdx
    40002a90:	48 8d 54 d2 f7       	lea    -0x9(%rdx,%rdx,8),%rdx
    40002a95:	48 39 d1             	cmp    %rdx,%rcx
    40002a98:	0f 8d 2e 05 00 00    	jge    40002fcc <fmt_fp+0x84c>
    40002a9e:	8d 90 00 40 02 00    	lea    0x24000(%rax),%edx
    40002aa4:	48 63 c2             	movslq %edx,%rax
    40002aa7:	89 d1                	mov    %edx,%ecx
    40002aa9:	48 69 c0 39 8e e3 38 	imul   $0x38e38e39,%rax,%rax
    40002ab0:	c1 f9 1f             	sar    $0x1f,%ecx
    40002ab3:	48 c1 f8 21          	sar    $0x21,%rax
    40002ab7:	29 c8                	sub    %ecx,%eax
    40002ab9:	48 63 c8             	movslq %eax,%rcx
    40002abc:	48 8d b4 8e 04 00 ff 	lea    -0xfffc(%rsi,%rcx,4),%rsi
    40002ac3:	ff 
    40002ac4:	8d 0c c0             	lea    (%rax,%rax,8),%ecx
    40002ac7:	29 ca                	sub    %ecx,%edx
    40002ac9:	8d 42 01             	lea    0x1(%rdx),%eax
    40002acc:	83 fa 08             	cmp    $0x8,%edx
    40002acf:	0f 84 9a 0e 00 00    	je     4000396f <fmt_fp+0x11ef>
    40002ad5:	b9 0a 00 00 00       	mov    $0xa,%ecx
    40002ada:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40002ae0:	8d 0c 89             	lea    (%rcx,%rcx,4),%ecx
    40002ae3:	83 c0 01             	add    $0x1,%eax
    40002ae6:	01 c9                	add    %ecx,%ecx
    40002ae8:	83 f8 09             	cmp    $0x9,%eax
    40002aeb:	75 f3                	jne    40002ae0 <fmt_fp+0x360>
    40002aed:	41 89 ca             	mov    %ecx,%r10d
    40002af0:	44 8b 0e             	mov    (%rsi),%r9d
    40002af3:	31 d2                	xor    %edx,%edx
    40002af5:	44 89 c8             	mov    %r9d,%eax
    40002af8:	41 f7 f2             	div    %r10d
    40002afb:	41 89 d0             	mov    %edx,%r8d
    40002afe:	85 d2                	test   %edx,%edx
    40002b00:	0f 85 62 04 00 00    	jne    40002f68 <fmt_fp+0x7e8>
    40002b06:	48 8d 56 04          	lea    0x4(%rsi),%rdx
    40002b0a:	48 39 d3             	cmp    %rdx,%rbx
    40002b0d:	0f 84 54 0e 00 00    	je     40003967 <fmt_fp+0x11e7>
    40002b13:	a8 01                	test   $0x1,%al
    40002b15:	0f 85 42 0d 00 00    	jne    4000385d <fmt_fp+0x10dd>
    40002b1b:	48 39 ee             	cmp    %rbp,%rsi
    40002b1e:	76 0c                	jbe    40002b2c <fmt_fp+0x3ac>
    40002b20:	81 f9 00 ca 9a 3b    	cmp    $0x3b9aca00,%ecx
    40002b26:	0f 84 6c 0d 00 00    	je     40003898 <fmt_fp+0x1118>
    40002b2c:	d9 05 42 45 00 00    	flds   0x4542(%rip)        # 40007074 <states+0x1d4>
    40002b32:	d9 05 40 45 00 00    	flds   0x4540(%rip)        # 40007078 <states+0x1d8>
    40002b38:	e9 50 04 00 00       	jmp    40002f8d <fmt_fp+0x80d>
    40002b3d:	0f 1f 00             	nopl   (%rax)
    40002b40:	8b 44 24 14          	mov    0x14(%rsp),%eax
    40002b44:	f6 c4 08             	test   $0x8,%ah
    40002b47:	0f 84 b9 02 00 00    	je     40002e06 <fmt_fp+0x686>
    40002b4d:	c7 44 24 20 01 00 00 	movl   $0x1,0x20(%rsp)
    40002b54:	00 
    40002b55:	48 c7 44 24 30 4b 6c 	movq   $0x40006c4b,0x30(%rsp)
    40002b5c:	00 40 
    40002b5e:	e9 88 fc ff ff       	jmp    400027eb <fmt_fp+0x6b>
    40002b63:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40002b68:	8b 44 24 50          	mov    0x50(%rsp),%eax
    40002b6c:	44 8b 7c 24 18       	mov    0x18(%rsp),%r15d
    40002b71:	44 8d 48 ff          	lea    -0x1(%rax),%r9d
    40002b75:	41 83 cf 20          	or     $0x20,%r15d
    40002b79:	44 89 4c 24 50       	mov    %r9d,0x50(%rsp)
    40002b7e:	41 83 ff 61          	cmp    $0x61,%r15d
    40002b82:	0f 85 ce fc ff ff    	jne    40002856 <fmt_fp+0xd6>
    40002b88:	48 8b 5c 24 30       	mov    0x30(%rsp),%rbx
    40002b8d:	f6 44 24 18 20       	testb  $0x20,0x18(%rsp)
    40002b92:	8b 4c 24 20          	mov    0x20(%rsp),%ecx
    40002b96:	d9 05 e4 44 00 00    	flds   0x44e4(%rip)        # 40007080 <states+0x1e0>
    40002b9c:	48 8d 43 09          	lea    0x9(%rbx),%rax
    40002ba0:	48 0f 44 c3          	cmove  %rbx,%rax
    40002ba4:	83 c1 02             	add    $0x2,%ecx
    40002ba7:	48 89 44 24 30       	mov    %rax,0x30(%rsp)
    40002bac:	41 83 fc 0e          	cmp    $0xe,%r12d
    40002bb0:	77 27                	ja     40002bd9 <fmt_fp+0x459>
    40002bb2:	d9 05 b8 44 00 00    	flds   0x44b8(%rip)        # 40007070 <states+0x1d0>
    40002bb8:	b8 0e 00 00 00       	mov    $0xe,%eax
    40002bbd:	44 29 e0             	sub    %r12d,%eax
    40002bc0:	d8 c9                	fmul   %st(1),%st
    40002bc2:	83 e8 01             	sub    $0x1,%eax
    40002bc5:	73 f9                	jae    40002bc0 <fmt_fp+0x440>
    40002bc7:	48 8b 44 24 30       	mov    0x30(%rsp),%rax
    40002bcc:	80 38 2d             	cmpb   $0x2d,(%rax)
    40002bcf:	0f 84 6b 0d 00 00    	je     40003940 <fmt_fp+0x11c0>
    40002bd5:	dc c3                	fadd   %st,%st(3)
    40002bd7:	de eb                	fsubrp %st,%st(3)
    40002bd9:	44 89 ce             	mov    %r9d,%esi
    40002bdc:	4c 8d 7c 24 60       	lea    0x60(%rsp),%r15
    40002be1:	49 b8 cd cc cc cc cc 	movabs $0xcccccccccccccccd,%r8
    40002be8:	cc cc cc 
    40002beb:	f7 de                	neg    %esi
    40002bed:	4c 89 ff             	mov    %r15,%rdi
    40002bf0:	41 0f 48 f1          	cmovs  %r9d,%esi
    40002bf4:	48 63 f6             	movslq %esi,%rsi
    40002bf7:	45 85 c9             	test   %r9d,%r9d
    40002bfa:	0f 84 7c 0c 00 00    	je     4000387c <fmt_fp+0x10fc>
    40002c00:	48 89 f0             	mov    %rsi,%rax
    40002c03:	48 83 ef 01          	sub    $0x1,%rdi
    40002c07:	49 f7 e0             	mul    %r8
    40002c0a:	48 89 f0             	mov    %rsi,%rax
    40002c0d:	48 c1 ea 03          	shr    $0x3,%rdx
    40002c11:	4c 8d 14 92          	lea    (%rdx,%rdx,4),%r10
    40002c15:	4d 01 d2             	add    %r10,%r10
    40002c18:	4c 29 d0             	sub    %r10,%rax
    40002c1b:	83 c0 30             	add    $0x30,%eax
    40002c1e:	88 07                	mov    %al,(%rdi)
    40002c20:	48 89 f0             	mov    %rsi,%rax
    40002c23:	48 89 d6             	mov    %rdx,%rsi
    40002c26:	48 83 f8 09          	cmp    $0x9,%rax
    40002c2a:	77 d4                	ja     40002c00 <fmt_fp+0x480>
    40002c2c:	4c 39 ff             	cmp    %r15,%rdi
    40002c2f:	0f 84 47 0c 00 00    	je     4000387c <fmt_fp+0x10fc>
    40002c35:	d9 ca                	fxch   %st(2)
    40002c37:	8b 5c 24 18          	mov    0x18(%rsp),%ebx
    40002c3b:	48 8d 47 fe          	lea    -0x2(%rdi),%rax
    40002c3f:	41 c1 f9 1f          	sar    $0x1f,%r9d
    40002c43:	48 89 44 24 20       	mov    %rax,0x20(%rsp)
    40002c48:	41 83 e1 02          	and    $0x2,%r9d
    40002c4c:	89 d8                	mov    %ebx,%eax
    40002c4e:	41 83 c1 2b          	add    $0x2b,%r9d
    40002c52:	89 de                	mov    %ebx,%esi
    40002c54:	83 c0 0f             	add    $0xf,%eax
    40002c57:	44 88 4f ff          	mov    %r9b,-0x1(%rdi)
    40002c5b:	44 8b 4c 24 14       	mov    0x14(%rsp),%r9d
    40002c60:	83 e6 20             	and    $0x20,%esi
    40002c63:	88 47 fe             	mov    %al,-0x2(%rdi)
    40002c66:	4c 89 f8             	mov    %r15,%rax
    40002c69:	bf 01 00 00 00       	mov    $0x1,%edi
    40002c6e:	d9 7c 24 4e          	fnstcw 0x4e(%rsp)
    40002c72:	41 83 e1 08          	and    $0x8,%r9d
    40002c76:	0f b7 54 24 4e       	movzwl 0x4e(%rsp),%edx
    40002c7b:	80 ce 0c             	or     $0xc,%dh
    40002c7e:	66 89 54 24 4c       	mov    %dx,0x4c(%rsp)
    40002c83:	eb 16                	jmp    40002c9b <fmt_fp+0x51b>
    40002c85:	0f 1f 00             	nopl   (%rax)
    40002c88:	4c 89 c0             	mov    %r8,%rax
    40002c8b:	d9 c1                	fld    %st(1)
    40002c8d:	d9 c9                	fxch   %st(1)
    40002c8f:	db e9                	fucomi %st(1),%st
    40002c91:	dd d9                	fstp   %st(1)
    40002c93:	7a 06                	jp     40002c9b <fmt_fp+0x51b>
    40002c95:	0f 84 83 0c 00 00    	je     4000391e <fmt_fp+0x119e>
    40002c9b:	d9 c0                	fld    %st(0)
    40002c9d:	d9 6c 24 4c          	fldcw  0x4c(%rsp)
    40002ca1:	db 5c 24 08          	fistpl 0x8(%rsp)
    40002ca5:	d9 6c 24 4e          	fldcw  0x4e(%rsp)
    40002ca9:	4c 8d 40 01          	lea    0x1(%rax),%r8
    40002cad:	db 44 24 08          	fildl  0x8(%rsp)
    40002cb1:	48 63 54 24 08       	movslq 0x8(%rsp),%rdx
    40002cb6:	0f b6 9a 80 6e 00 40 	movzbl 0x40006e80(%rdx),%ebx
    40002cbd:	de e9                	fsubrp %st,%st(1)
    40002cbf:	09 f3                	or     %esi,%ebx
    40002cc1:	88 18                	mov    %bl,(%rax)
    40002cc3:	d8 ca                	fmul   %st(2),%st
    40002cc5:	4c 39 f8             	cmp    %r15,%rax
    40002cc8:	75 be                	jne    40002c88 <fmt_fp+0x508>
    40002cca:	d9 c1                	fld    %st(1)
    40002ccc:	df e9                	fucomip %st(1),%st
    40002cce:	0f 9a c2             	setp   %dl
    40002cd1:	0f 45 d7             	cmovne %edi,%edx
    40002cd4:	84 d2                	test   %dl,%dl
    40002cd6:	0f 85 cc 06 00 00    	jne    400033a8 <fmt_fp+0xc28>
    40002cdc:	45 85 e4             	test   %r12d,%r12d
    40002cdf:	0f 8f c3 06 00 00    	jg     400033a8 <fmt_fp+0xc28>
    40002ce5:	45 85 c9             	test   %r9d,%r9d
    40002ce8:	0f 85 ba 06 00 00    	jne    400033a8 <fmt_fp+0xc28>
    40002cee:	dd d8                	fstp   %st(0)
    40002cf0:	dd d8                	fstp   %st(0)
    40002cf2:	dd d8                	fstp   %st(0)
    40002cf4:	4c 89 fd             	mov    %r15,%rbp
    40002cf7:	b8 fd ff ff 7f       	mov    $0x7ffffffd,%eax
    40002cfc:	48 2b 6c 24 20       	sub    0x20(%rsp),%rbp
    40002d01:	4c 63 d1             	movslq %ecx,%r10
    40002d04:	48 29 e8             	sub    %rbp,%rax
    40002d07:	49 63 d4             	movslq %r12d,%rdx
    40002d0a:	4c 29 d0             	sub    %r10,%rax
    40002d0d:	48 39 c2             	cmp    %rax,%rdx
    40002d10:	0f 8f b7 06 00 00    	jg     400033cd <fmt_fp+0xc4d>
    40002d16:	4d 29 f8             	sub    %r15,%r8
    40002d19:	42 8d 5c 05 00       	lea    0x0(%rbp,%r8,1),%ebx
    40002d1e:	45 85 e4             	test   %r12d,%r12d
    40002d21:	74 0d                	je     40002d30 <fmt_fp+0x5b0>
    40002d23:	49 8d 40 ff          	lea    -0x1(%r8),%rax
    40002d27:	48 39 c2             	cmp    %rax,%rdx
    40002d2a:	0f 8d 7d 0b 00 00    	jge    400038ad <fmt_fp+0x112d>
    40002d30:	45 31 f6             	xor    %r14d,%r14d
    40002d33:	8b 54 24 38          	mov    0x38(%rsp),%edx
    40002d37:	01 cb                	add    %ecx,%ebx
    40002d39:	39 d3                	cmp    %edx,%ebx
    40002d3b:	41 0f 9d c4          	setge  %r12b
    40002d3f:	f7 44 24 14 00 20 01 	testl  $0x12000,0x14(%rsp)
    40002d46:	00 
    40002d47:	75 28                	jne    40002d71 <fmt_fp+0x5f1>
    40002d49:	45 84 e4             	test   %r12b,%r12b
    40002d4c:	75 23                	jne    40002d71 <fmt_fp+0x5f1>
    40002d4e:	89 d9                	mov    %ebx,%ecx
    40002d50:	be 20 00 00 00       	mov    $0x20,%esi
    40002d55:	4c 89 ef             	mov    %r13,%rdi
    40002d58:	4c 89 54 24 18       	mov    %r10,0x18(%rsp)
    40002d5d:	4c 89 44 24 08       	mov    %r8,0x8(%rsp)
    40002d62:	e8 59 f9 ff ff       	call   400026c0 <pad.part.0>
    40002d67:	4c 8b 54 24 18       	mov    0x18(%rsp),%r10
    40002d6c:	4c 8b 44 24 08       	mov    0x8(%rsp),%r8
    40002d71:	41 f6 45 00 20       	testb  $0x20,0x0(%r13)
    40002d76:	0f 84 a5 0a 00 00    	je     40003821 <fmt_fp+0x10a1>
    40002d7c:	8b 44 24 14          	mov    0x14(%rsp),%eax
    40002d80:	35 00 00 01 00       	xor    $0x10000,%eax
    40002d85:	a9 00 20 01 00       	test   $0x12000,%eax
    40002d8a:	0f 85 53 07 00 00    	jne    400034e3 <fmt_fp+0xd63>
    40002d90:	45 84 e4             	test   %r12b,%r12b
    40002d93:	0f 85 4a 07 00 00    	jne    400034e3 <fmt_fp+0xd63>
    40002d99:	8b 54 24 38          	mov    0x38(%rsp),%edx
    40002d9d:	89 d9                	mov    %ebx,%ecx
    40002d9f:	be 30 00 00 00       	mov    $0x30,%esi
    40002da4:	4c 89 ef             	mov    %r13,%rdi
    40002da7:	4c 89 44 24 08       	mov    %r8,0x8(%rsp)
    40002dac:	e8 0f f9 ff ff       	call   400026c0 <pad.part.0>
    40002db1:	4c 8b 44 24 08       	mov    0x8(%rsp),%r8
    40002db6:	41 f6 45 00 20       	testb  $0x20,0x0(%r13)
    40002dbb:	0f 85 22 07 00 00    	jne    400034e3 <fmt_fp+0xd63>
    40002dc1:	4c 89 ea             	mov    %r13,%rdx
    40002dc4:	4c 89 c6             	mov    %r8,%rsi
    40002dc7:	4c 89 ff             	mov    %r15,%rdi
    40002dca:	e8 61 2b 00 00       	call   40005930 <__fwritex>
    40002dcf:	45 85 f6             	test   %r14d,%r14d
    40002dd2:	7e 12                	jle    40002de6 <fmt_fp+0x666>
    40002dd4:	31 c9                	xor    %ecx,%ecx
    40002dd6:	44 89 f2             	mov    %r14d,%edx
    40002dd9:	be 30 00 00 00       	mov    $0x30,%esi
    40002dde:	4c 89 ef             	mov    %r13,%rdi
    40002de1:	e8 da f8 ff ff       	call   400026c0 <pad.part.0>
    40002de6:	41 f6 45 00 20       	testb  $0x20,0x0(%r13)
    40002deb:	0f 85 d7 00 00 00    	jne    40002ec8 <fmt_fp+0x748>
    40002df1:	48 8b 7c 24 20       	mov    0x20(%rsp),%rdi
    40002df6:	4c 89 ea             	mov    %r13,%rdx
    40002df9:	48 89 ee             	mov    %rbp,%rsi
    40002dfc:	e8 2f 2b 00 00       	call   40005930 <__fwritex>
    40002e01:	e9 c2 00 00 00       	jmp    40002ec8 <fmt_fp+0x748>
    40002e06:	a8 01                	test   $0x1,%al
    40002e08:	ba 4e 6c 00 40       	mov    $0x40006c4e,%edx
    40002e0d:	b8 49 6c 00 40       	mov    $0x40006c49,%eax
    40002e12:	48 0f 45 c2          	cmovne %rdx,%rax
    40002e16:	48 89 44 24 30       	mov    %rax,0x30(%rsp)
    40002e1b:	0f 95 c0             	setne  %al
    40002e1e:	0f b6 c0             	movzbl %al,%eax
    40002e21:	89 44 24 20          	mov    %eax,0x20(%rsp)
    40002e25:	e9 c1 f9 ff ff       	jmp    400027eb <fmt_fp+0x6b>
    40002e2a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40002e30:	44 89 c9             	mov    %r9d,%ecx
    40002e33:	8b 45 00             	mov    0x0(%rbp),%eax
    40002e36:	45 31 c9             	xor    %r9d,%r9d
    40002e39:	f7 d9                	neg    %ecx
    40002e3b:	48 39 dd             	cmp    %rbx,%rbp
    40002e3e:	0f 82 93 fb ff ff    	jb     400029d7 <fmt_fp+0x257>
    40002e44:	48 8d 55 04          	lea    0x4(%rbp),%rdx
    40002e48:	85 c0                	test   %eax,%eax
    40002e4a:	48 0f 44 ea          	cmove  %rdx,%rbp
    40002e4e:	e9 d9 fb ff ff       	jmp    40002a2c <fmt_fp+0x2ac>
    40002e53:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40002e58:	8b 4c 24 18          	mov    0x18(%rsp),%ecx
    40002e5c:	b8 5b 6c 00 40       	mov    $0x40006c5b,%eax
    40002e61:	ba 5f 6c 00 40       	mov    $0x40006c5f,%edx
    40002e66:	db ac 24 70 1d 00 00 	fldt   0x1d70(%rsp)
    40002e6d:	83 e1 20             	and    $0x20,%ecx
    40002e70:	48 0f 45 d0          	cmovne %rax,%rdx
    40002e74:	df e8                	fucomip %st(0),%st
    40002e76:	48 89 d5             	mov    %rdx,%rbp
    40002e79:	7b 13                	jnp    40002e8e <fmt_fp+0x70e>
    40002e7b:	85 c9                	test   %ecx,%ecx
    40002e7d:	b8 67 6c 00 40       	mov    $0x40006c67,%eax
    40002e82:	ba 63 6c 00 40       	mov    $0x40006c63,%edx
    40002e87:	48 0f 45 d0          	cmovne %rax,%rdx
    40002e8b:	48 89 d5             	mov    %rdx,%rbp
    40002e8e:	8b 44 24 20          	mov    0x20(%rsp),%eax
    40002e92:	8b 54 24 38          	mov    0x38(%rsp),%edx
    40002e96:	8d 58 03             	lea    0x3(%rax),%ebx
    40002e99:	39 d3                	cmp    %edx,%ebx
    40002e9b:	41 0f 9d c4          	setge  %r12b
    40002e9f:	f7 44 24 14 00 20 00 	testl  $0x2000,0x14(%rsp)
    40002ea6:	00 
    40002ea7:	75 14                	jne    40002ebd <fmt_fp+0x73d>
    40002ea9:	45 84 e4             	test   %r12b,%r12b
    40002eac:	75 0f                	jne    40002ebd <fmt_fp+0x73d>
    40002eae:	89 d9                	mov    %ebx,%ecx
    40002eb0:	be 20 00 00 00       	mov    $0x20,%esi
    40002eb5:	4c 89 ef             	mov    %r13,%rdi
    40002eb8:	e8 03 f8 ff ff       	call   400026c0 <pad.part.0>
    40002ebd:	41 f6 45 00 20       	testb  $0x20,0x0(%r13)
    40002ec2:	0f 84 1f 05 00 00    	je     400033e7 <fmt_fp+0xc67>
    40002ec8:	8b 44 24 14          	mov    0x14(%rsp),%eax
    40002ecc:	80 f4 20             	xor    $0x20,%ah
    40002ecf:	a9 00 20 01 00       	test   $0x12000,%eax
    40002ed4:	75 18                	jne    40002eee <fmt_fp+0x76e>
    40002ed6:	45 84 e4             	test   %r12b,%r12b
    40002ed9:	75 13                	jne    40002eee <fmt_fp+0x76e>
    40002edb:	8b 54 24 38          	mov    0x38(%rsp),%edx
    40002edf:	89 d9                	mov    %ebx,%ecx
    40002ee1:	be 20 00 00 00       	mov    $0x20,%esi
    40002ee6:	4c 89 ef             	mov    %r13,%rdi
    40002ee9:	e8 d2 f7 ff ff       	call   400026c0 <pad.part.0>
    40002eee:	8b 44 24 38          	mov    0x38(%rsp),%eax
    40002ef2:	39 c3                	cmp    %eax,%ebx
    40002ef4:	0f 4d c3             	cmovge %ebx,%eax
    40002ef7:	48 81 c4 38 1d 00 00 	add    $0x1d38,%rsp
    40002efe:	5b                   	pop    %rbx
    40002eff:	5d                   	pop    %rbp
    40002f00:	41 5c                	pop    %r12
    40002f02:	41 5d                	pop    %r13
    40002f04:	41 5e                	pop    %r14
    40002f06:	41 5f                	pop    %r15
    40002f08:	c3                   	ret    
    40002f09:	48 8b 44 24 28       	mov    0x28(%rsp),%rax
    40002f0e:	31 c9                	xor    %ecx,%ecx
    40002f10:	41 83 ff 66          	cmp    $0x66,%r15d
    40002f14:	8b 55 00             	mov    0x0(%rbp),%edx
    40002f17:	0f 95 c1             	setne  %cl
    40002f1a:	48 29 e8             	sub    %rbp,%rax
    40002f1d:	48 c1 f8 02          	sar    $0x2,%rax
    40002f21:	8d 3c c0             	lea    (%rax,%rax,8),%edi
    40002f24:	b8 0a 00 00 00       	mov    $0xa,%eax
    40002f29:	83 fa 09             	cmp    $0x9,%edx
    40002f2c:	76 0e                	jbe    40002f3c <fmt_fp+0x7bc>
    40002f2e:	66 90                	xchg   %ax,%ax
    40002f30:	8d 04 80             	lea    (%rax,%rax,4),%eax
    40002f33:	83 c7 01             	add    $0x1,%edi
    40002f36:	01 c0                	add    %eax,%eax
    40002f38:	39 d0                	cmp    %edx,%eax
    40002f3a:	76 f4                	jbe    40002f30 <fmt_fp+0x7b0>
    40002f3c:	0f af cf             	imul   %edi,%ecx
    40002f3f:	44 89 e0             	mov    %r12d,%eax
    40002f42:	29 c8                	sub    %ecx,%eax
    40002f44:	e9 26 fb ff ff       	jmp    40002a6f <fmt_fp+0x2ef>
    40002f49:	d9 c9                	fxch   %st(1)
    40002f4b:	eb 05                	jmp    40002f52 <fmt_fp+0x7d2>
    40002f4d:	0f 1f 00             	nopl   (%rax)
    40002f50:	d9 c9                	fxch   %st(1)
    40002f52:	41 83 e9 1c          	sub    $0x1c,%r9d
    40002f56:	d8 0d 28 41 00 00    	fmuls  0x4128(%rip)        # 40007084 <states+0x1e4>
    40002f5c:	d9 c9                	fxch   %st(1)
    40002f5e:	44 89 4c 24 50       	mov    %r9d,0x50(%rsp)
    40002f63:	e9 10 f9 ff ff       	jmp    40002878 <fmt_fp+0xf8>
    40002f68:	a8 01                	test   $0x1,%al
    40002f6a:	0f 84 1b 09 00 00    	je     4000388b <fmt_fp+0x110b>
    40002f70:	db 2d 1a 41 00 00    	fldt   0x411a(%rip)        # 40007090 <states+0x1f0>
    40002f76:	d1 f9                	sar    %ecx
    40002f78:	44 39 c1             	cmp    %r8d,%ecx
    40002f7b:	0f 87 e2 08 00 00    	ja     40003863 <fmt_fp+0x10e3>
    40002f81:	0f 84 cc 09 00 00    	je     40003953 <fmt_fp+0x11d3>
    40002f87:	d9 05 ef 40 00 00    	flds   0x40ef(%rip)        # 4000707c <states+0x1dc>
    40002f8d:	8b 54 24 20          	mov    0x20(%rsp),%edx
    40002f91:	85 d2                	test   %edx,%edx
    40002f93:	74 12                	je     40002fa7 <fmt_fp+0x827>
    40002f95:	48 8b 44 24 30       	mov    0x30(%rsp),%rax
    40002f9a:	80 38 2d             	cmpb   $0x2d,(%rax)
    40002f9d:	75 08                	jne    40002fa7 <fmt_fp+0x827>
    40002f9f:	d9 c9                	fxch   %st(1)
    40002fa1:	d9 e0                	fchs   
    40002fa3:	d9 c9                	fxch   %st(1)
    40002fa5:	d9 e0                	fchs   
    40002fa7:	d8 c1                	fadd   %st(1),%st
    40002fa9:	44 89 c8             	mov    %r9d,%eax
    40002fac:	44 29 c0             	sub    %r8d,%eax
    40002faf:	df e9                	fucomip %st(1),%st
    40002fb1:	dd d8                	fstp   %st(0)
    40002fb3:	0f 8a ee 07 00 00    	jp     400037a7 <fmt_fp+0x1027>
    40002fb9:	0f 85 e8 07 00 00    	jne    400037a7 <fmt_fp+0x1027>
    40002fbf:	89 06                	mov    %eax,(%rsi)
    40002fc1:	48 8d 56 04          	lea    0x4(%rsi),%rdx
    40002fc5:	48 39 d3             	cmp    %rdx,%rbx
    40002fc8:	48 0f 47 da          	cmova  %rdx,%rbx
    40002fcc:	48 39 dd             	cmp    %rbx,%rbp
    40002fcf:	72 10                	jb     40002fe1 <fmt_fp+0x861>
    40002fd1:	eb 15                	jmp    40002fe8 <fmt_fp+0x868>
    40002fd3:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40002fd8:	48 83 eb 04          	sub    $0x4,%rbx
    40002fdc:	48 39 eb             	cmp    %rbp,%rbx
    40002fdf:	76 07                	jbe    40002fe8 <fmt_fp+0x868>
    40002fe1:	8b 43 fc             	mov    -0x4(%rbx),%eax
    40002fe4:	85 c0                	test   %eax,%eax
    40002fe6:	74 f0                	je     40002fd8 <fmt_fp+0x858>
    40002fe8:	41 83 ff 67          	cmp    $0x67,%r15d
    40002fec:	0f 84 37 04 00 00    	je     40003429 <fmt_fp+0xca9>
    40002ff2:	45 85 e4             	test   %r12d,%r12d
    40002ff5:	0f 85 ba 03 00 00    	jne    400033b5 <fmt_fp+0xc35>
    40002ffb:	44 8b 74 24 14       	mov    0x14(%rsp),%r14d
    40003000:	b8 01 00 00 00       	mov    $0x1,%eax
    40003005:	41 c1 ee 03          	shr    $0x3,%r14d
    40003009:	41 83 e6 01          	and    $0x1,%r14d
    4000300d:	44 8b 44 24 18       	mov    0x18(%rsp),%r8d
    40003012:	41 01 c6             	add    %eax,%r14d
    40003015:	b8 ff ff ff 7f       	mov    $0x7fffffff,%eax
    4000301a:	44 29 f0             	sub    %r14d,%eax
    4000301d:	41 83 c8 20          	or     $0x20,%r8d
    40003021:	4c 63 d8             	movslq %eax,%r11
    40003024:	41 83 f8 66          	cmp    $0x66,%r8d
    40003028:	0f 84 c3 04 00 00    	je     400034f1 <fmt_fp+0xd71>
    4000302e:	89 fe                	mov    %edi,%esi
    40003030:	4c 8d 7c 24 60       	lea    0x60(%rsp),%r15
    40003035:	f7 de                	neg    %esi
    40003037:	4c 89 f9             	mov    %r15,%rcx
    4000303a:	0f 48 f7             	cmovs  %edi,%esi
    4000303d:	48 63 f6             	movslq %esi,%rsi
    40003040:	85 ff                	test   %edi,%edi
    40003042:	74 4c                	je     40003090 <fmt_fp+0x910>
    40003044:	49 b9 cd cc cc cc cc 	movabs $0xcccccccccccccccd,%r9
    4000304b:	cc cc cc 
    4000304e:	66 90                	xchg   %ax,%ax
    40003050:	48 89 f0             	mov    %rsi,%rax
    40003053:	48 83 e9 01          	sub    $0x1,%rcx
    40003057:	49 f7 e1             	mul    %r9
    4000305a:	48 89 f0             	mov    %rsi,%rax
    4000305d:	48 c1 ea 03          	shr    $0x3,%rdx
    40003061:	4c 8d 14 92          	lea    (%rdx,%rdx,4),%r10
    40003065:	4d 01 d2             	add    %r10,%r10
    40003068:	4c 29 d0             	sub    %r10,%rax
    4000306b:	83 c0 30             	add    $0x30,%eax
    4000306e:	88 01                	mov    %al,(%rcx)
    40003070:	48 89 f0             	mov    %rsi,%rax
    40003073:	48 89 d6             	mov    %rdx,%rsi
    40003076:	48 83 f8 09          	cmp    $0x9,%rax
    4000307a:	77 d4                	ja     40003050 <fmt_fp+0x8d0>
    4000307c:	4c 89 f8             	mov    %r15,%rax
    4000307f:	48 29 c8             	sub    %rcx,%rax
    40003082:	48 83 f8 01          	cmp    $0x1,%rax
    40003086:	7f 1b                	jg     400030a3 <fmt_fp+0x923>
    40003088:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4000308f:	00 
    40003090:	48 83 e9 01          	sub    $0x1,%rcx
    40003094:	4c 89 f8             	mov    %r15,%rax
    40003097:	c6 01 30             	movb   $0x30,(%rcx)
    4000309a:	48 29 c8             	sub    %rcx,%rax
    4000309d:	48 83 f8 01          	cmp    $0x1,%rax
    400030a1:	7e ed                	jle    40003090 <fmt_fp+0x910>
    400030a3:	c1 ff 1f             	sar    $0x1f,%edi
    400030a6:	48 8d 41 fe          	lea    -0x2(%rcx),%rax
    400030aa:	83 e7 02             	and    $0x2,%edi
    400030ad:	49 29 c7             	sub    %rax,%r15
    400030b0:	48 89 44 24 40       	mov    %rax,0x40(%rsp)
    400030b5:	83 c7 2b             	add    $0x2b,%edi
    400030b8:	40 88 79 ff          	mov    %dil,-0x1(%rcx)
    400030bc:	0f b6 7c 24 18       	movzbl 0x18(%rsp),%edi
    400030c1:	40 88 79 fe          	mov    %dil,-0x2(%rcx)
    400030c5:	4d 39 df             	cmp    %r11,%r15
    400030c8:	0f 8f ff 02 00 00    	jg     400033cd <fmt_fp+0xc4d>
    400030ce:	45 01 fe             	add    %r15d,%r14d
    400030d1:	8b 4c 24 20          	mov    0x20(%rsp),%ecx
    400030d5:	89 c8                	mov    %ecx,%eax
    400030d7:	f7 d8                	neg    %eax
    400030d9:	19 c0                	sbb    %eax,%eax
    400030db:	05 ff ff ff 7f       	add    $0x7fffffff,%eax
    400030e0:	44 39 f0             	cmp    %r14d,%eax
    400030e3:	0f 8c e4 02 00 00    	jl     400033cd <fmt_fp+0xc4d>
    400030e9:	8b 54 24 38          	mov    0x38(%rsp),%edx
    400030ed:	41 01 ce             	add    %ecx,%r14d
    400030f0:	44 89 74 24 48       	mov    %r14d,0x48(%rsp)
    400030f5:	44 89 f1             	mov    %r14d,%ecx
    400030f8:	41 39 d6             	cmp    %edx,%r14d
    400030fb:	0f 9d 44 24 3c       	setge  0x3c(%rsp)
    40003100:	0f b6 44 24 3c       	movzbl 0x3c(%rsp),%eax
    40003105:	f7 44 24 14 00 20 01 	testl  $0x12000,0x14(%rsp)
    4000310c:	00 
    4000310d:	75 1b                	jne    4000312a <fmt_fp+0x9aa>
    4000310f:	84 c0                	test   %al,%al
    40003111:	75 17                	jne    4000312a <fmt_fp+0x9aa>
    40003113:	be 20 00 00 00       	mov    $0x20,%esi
    40003118:	4c 89 ef             	mov    %r13,%rdi
    4000311b:	44 89 44 24 08       	mov    %r8d,0x8(%rsp)
    40003120:	e8 9b f5 ff ff       	call   400026c0 <pad.part.0>
    40003125:	44 8b 44 24 08       	mov    0x8(%rsp),%r8d
    4000312a:	41 f6 45 00 20       	testb  $0x20,0x0(%r13)
    4000312f:	0f 84 20 06 00 00    	je     40003755 <fmt_fp+0xfd5>
    40003135:	8b 44 24 14          	mov    0x14(%rsp),%eax
    40003139:	35 00 00 01 00       	xor    $0x10000,%eax
    4000313e:	a9 00 20 01 00       	test   $0x12000,%eax
    40003143:	75 26                	jne    4000316b <fmt_fp+0x9eb>
    40003145:	80 7c 24 3c 00       	cmpb   $0x0,0x3c(%rsp)
    4000314a:	75 1f                	jne    4000316b <fmt_fp+0x9eb>
    4000314c:	8b 4c 24 48          	mov    0x48(%rsp),%ecx
    40003150:	8b 54 24 38          	mov    0x38(%rsp),%edx
    40003154:	be 30 00 00 00       	mov    $0x30,%esi
    40003159:	4c 89 ef             	mov    %r13,%rdi
    4000315c:	44 89 44 24 08       	mov    %r8d,0x8(%rsp)
    40003161:	e8 5a f5 ff ff       	call   400026c0 <pad.part.0>
    40003166:	44 8b 44 24 08       	mov    0x8(%rsp),%r8d
    4000316b:	41 83 f8 66          	cmp    $0x66,%r8d
    4000316f:	0f 84 94 03 00 00    	je     40003509 <fmt_fp+0xd89>
    40003175:	48 8d 45 04          	lea    0x4(%rbp),%rax
    40003179:	48 39 dd             	cmp    %rbx,%rbp
    4000317c:	48 0f 43 d8          	cmovae %rax,%rbx
    40003180:	45 85 e4             	test   %r12d,%r12d
    40003183:	0f 88 29 01 00 00    	js     400032b2 <fmt_fp+0xb32>
    40003189:	48 39 eb             	cmp    %rbp,%rbx
    4000318c:	0f 86 20 01 00 00    	jbe    400032b2 <fmt_fp+0xb32>
    40003192:	4c 8d 4c 24 69       	lea    0x69(%rsp),%r9
    40003197:	48 89 5c 24 08       	mov    %rbx,0x8(%rsp)
    4000319c:	49 89 ef             	mov    %rbp,%r15
    4000319f:	4c 89 eb             	mov    %r13,%rbx
    400031a2:	48 89 6c 24 18       	mov    %rbp,0x18(%rsp)
    400031a7:	4d 63 ec             	movslq %r12d,%r13
    400031aa:	4c 89 cd             	mov    %r9,%rbp
    400031ad:	49 be cd cc cc cc cc 	movabs $0xcccccccccccccccd,%r14
    400031b4:	cc cc cc 
    400031b7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    400031be:	00 00 
    400031c0:	41 8b 0f             	mov    (%r15),%ecx
    400031c3:	48 89 ef             	mov    %rbp,%rdi
    400031c6:	48 85 c9             	test   %rcx,%rcx
    400031c9:	0f 84 3d 01 00 00    	je     4000330c <fmt_fp+0xb8c>
    400031cf:	90                   	nop
    400031d0:	48 89 c8             	mov    %rcx,%rax
    400031d3:	49 89 fc             	mov    %rdi,%r12
    400031d6:	48 8d 7f ff          	lea    -0x1(%rdi),%rdi
    400031da:	49 f7 e6             	mul    %r14
    400031dd:	48 89 c8             	mov    %rcx,%rax
    400031e0:	48 c1 ea 03          	shr    $0x3,%rdx
    400031e4:	48 8d 34 92          	lea    (%rdx,%rdx,4),%rsi
    400031e8:	48 01 f6             	add    %rsi,%rsi
    400031eb:	48 29 f0             	sub    %rsi,%rax
    400031ee:	83 c0 30             	add    $0x30,%eax
    400031f1:	41 88 44 24 ff       	mov    %al,-0x1(%r12)
    400031f6:	48 89 c8             	mov    %rcx,%rax
    400031f9:	48 89 d1             	mov    %rdx,%rcx
    400031fc:	48 83 f8 09          	cmp    $0x9,%rax
    40003200:	77 ce                	ja     400031d0 <fmt_fp+0xa50>
    40003202:	8b 0b                	mov    (%rbx),%ecx
    40003204:	48 8d 54 24 60       	lea    0x60(%rsp),%rdx
    40003209:	48 39 ef             	cmp    %rbp,%rdi
    4000320c:	0f 84 fc 00 00 00    	je     4000330e <fmt_fp+0xb8e>
    40003212:	4c 39 7c 24 18       	cmp    %r15,0x18(%rsp)
    40003217:	0f 84 0e 01 00 00    	je     4000332b <fmt_fp+0xbab>
    4000321d:	48 39 d7             	cmp    %rdx,%rdi
    40003220:	0f 86 f3 01 00 00    	jbe    40003419 <fmt_fp+0xc99>
    40003226:	48 89 f8             	mov    %rdi,%rax
    40003229:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40003230:	48 83 e8 01          	sub    $0x1,%rax
    40003234:	c6 00 30             	movb   $0x30,(%rax)
    40003237:	48 39 d0             	cmp    %rdx,%rax
    4000323a:	77 f4                	ja     40003230 <fmt_fp+0xab0>
    4000323c:	48 8d 47 ff          	lea    -0x1(%rdi),%rax
    40003240:	49 89 d4             	mov    %rdx,%r12
    40003243:	45 89 eb             	mov    %r13d,%r11d
    40003246:	49 29 fc             	sub    %rdi,%r12
    40003249:	48 39 c2             	cmp    %rax,%rdx
    4000324c:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    40003253:	4c 0f 47 e0          	cmova  %rax,%r12
    40003257:	89 c8                	mov    %ecx,%eax
    40003259:	83 e0 20             	and    $0x20,%eax
    4000325c:	49 01 fc             	add    %rdi,%r12
    4000325f:	48 89 e9             	mov    %rbp,%rcx
    40003262:	4c 29 e1             	sub    %r12,%rcx
    40003265:	85 c0                	test   %eax,%eax
    40003267:	75 29                	jne    40003292 <fmt_fp+0xb12>
    40003269:	49 39 cd             	cmp    %rcx,%r13
    4000326c:	4c 89 ee             	mov    %r13,%rsi
    4000326f:	48 89 da             	mov    %rbx,%rdx
    40003272:	4c 89 e7             	mov    %r12,%rdi
    40003275:	48 0f 4f f1          	cmovg  %rcx,%rsi
    40003279:	44 89 5c 24 28       	mov    %r11d,0x28(%rsp)
    4000327e:	48 89 4c 24 20       	mov    %rcx,0x20(%rsp)
    40003283:	e8 a8 26 00 00       	call   40005930 <__fwritex>
    40003288:	44 8b 5c 24 28       	mov    0x28(%rsp),%r11d
    4000328d:	48 8b 4c 24 20       	mov    0x20(%rsp),%rcx
    40003292:	41 29 cb             	sub    %ecx,%r11d
    40003295:	49 83 c7 04          	add    $0x4,%r15
    40003299:	4d 63 eb             	movslq %r11d,%r13
    4000329c:	4c 39 7c 24 08       	cmp    %r15,0x8(%rsp)
    400032a1:	76 09                	jbe    400032ac <fmt_fp+0xb2c>
    400032a3:	45 85 ed             	test   %r13d,%r13d
    400032a6:	0f 89 14 ff ff ff    	jns    400031c0 <fmt_fp+0xa40>
    400032ac:	45 89 ec             	mov    %r13d,%r12d
    400032af:	49 89 dd             	mov    %rbx,%r13
    400032b2:	41 8d 54 24 12       	lea    0x12(%r12),%edx
    400032b7:	83 fa 12             	cmp    $0x12,%edx
    400032ba:	7e 12                	jle    400032ce <fmt_fp+0xb4e>
    400032bc:	b9 12 00 00 00       	mov    $0x12,%ecx
    400032c1:	be 30 00 00 00       	mov    $0x30,%esi
    400032c6:	4c 89 ef             	mov    %r13,%rdi
    400032c9:	e8 f2 f3 ff ff       	call   400026c0 <pad.part.0>
    400032ce:	41 f6 45 00 20       	testb  $0x20,0x0(%r13)
    400032d3:	0f 84 b4 04 00 00    	je     4000378d <fmt_fp+0x100d>
    400032d9:	8b 44 24 14          	mov    0x14(%rsp),%eax
    400032dd:	80 f4 20             	xor    $0x20,%ah
    400032e0:	a9 00 20 01 00       	test   $0x12000,%eax
    400032e5:	75 1c                	jne    40003303 <fmt_fp+0xb83>
    400032e7:	80 7c 24 3c 00       	cmpb   $0x0,0x3c(%rsp)
    400032ec:	75 15                	jne    40003303 <fmt_fp+0xb83>
    400032ee:	8b 4c 24 48          	mov    0x48(%rsp),%ecx
    400032f2:	8b 54 24 38          	mov    0x38(%rsp),%edx
    400032f6:	be 20 00 00 00       	mov    $0x20,%esi
    400032fb:	4c 89 ef             	mov    %r13,%rdi
    400032fe:	e8 bd f3 ff ff       	call   400026c0 <pad.part.0>
    40003303:	8b 5c 24 48          	mov    0x48(%rsp),%ebx
    40003307:	e9 e2 fb ff ff       	jmp    40002eee <fmt_fp+0x76e>
    4000330c:	8b 0b                	mov    (%rbx),%ecx
    4000330e:	c6 44 24 68 30       	movb   $0x30,0x68(%rsp)
    40003313:	48 8d 7c 24 68       	lea    0x68(%rsp),%rdi
    40003318:	48 8d 54 24 60       	lea    0x60(%rsp),%rdx
    4000331d:	49 89 ec             	mov    %rbp,%r12
    40003320:	4c 39 7c 24 18       	cmp    %r15,0x18(%rsp)
    40003325:	0f 85 fb fe ff ff    	jne    40003226 <fmt_fp+0xaa6>
    4000332b:	83 e1 20             	and    $0x20,%ecx
    4000332e:	74 2b                	je     4000335b <fmt_fp+0xbdb>
    40003330:	8b 44 24 14          	mov    0x14(%rsp),%eax
    40003334:	45 89 eb             	mov    %r13d,%r11d
    40003337:	83 e0 08             	and    $0x8,%eax
    4000333a:	44 09 e8             	or     %r13d,%eax
    4000333d:	75 11                	jne    40003350 <fmt_fp+0xbd0>
    4000333f:	48 89 e9             	mov    %rbp,%rcx
    40003342:	45 31 db             	xor    %r11d,%r11d
    40003345:	4c 29 e1             	sub    %r12,%rcx
    40003348:	e9 45 ff ff ff       	jmp    40003292 <fmt_fp+0xb12>
    4000334d:	45 89 eb             	mov    %r13d,%r11d
    40003350:	48 89 e9             	mov    %rbp,%rcx
    40003353:	4c 29 e1             	sub    %r12,%rcx
    40003356:	e9 37 ff ff ff       	jmp    40003292 <fmt_fp+0xb12>
    4000335b:	48 89 da             	mov    %rbx,%rdx
    4000335e:	be 01 00 00 00       	mov    $0x1,%esi
    40003363:	e8 c8 25 00 00       	call   40005930 <__fwritex>
    40003368:	44 8b 5c 24 14       	mov    0x14(%rsp),%r11d
    4000336d:	8b 03                	mov    (%rbx),%eax
    4000336f:	41 83 e3 08          	and    $0x8,%r11d
    40003373:	83 e0 20             	and    $0x20,%eax
    40003376:	45 09 eb             	or     %r13d,%r11d
    40003379:	0f 84 e0 fe ff ff    	je     4000325f <fmt_fp+0xadf>
    4000337f:	85 c0                	test   %eax,%eax
    40003381:	75 ca                	jne    4000334d <fmt_fp+0xbcd>
    40003383:	48 89 da             	mov    %rbx,%rdx
    40003386:	be 01 00 00 00       	mov    $0x1,%esi
    4000338b:	bf 6b 6c 00 40       	mov    $0x40006c6b,%edi
    40003390:	e8 9b 25 00 00       	call   40005930 <__fwritex>
    40003395:	8b 03                	mov    (%rbx),%eax
    40003397:	45 89 eb             	mov    %r13d,%r11d
    4000339a:	83 e0 20             	and    $0x20,%eax
    4000339d:	e9 bd fe ff ff       	jmp    4000325f <fmt_fp+0xadf>
    400033a2:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    400033a8:	c6 40 01 2e          	movb   $0x2e,0x1(%rax)
    400033ac:	48 83 c0 02          	add    $0x2,%rax
    400033b0:	e9 d6 f8 ff ff       	jmp    40002c8b <fmt_fp+0x50b>
    400033b5:	41 8d 44 24 01       	lea    0x1(%r12),%eax
    400033ba:	41 be 01 00 00 00    	mov    $0x1,%r14d
    400033c0:	41 81 fc fd ff ff 7f 	cmp    $0x7ffffffd,%r12d
    400033c7:	0f 8e 40 fc ff ff    	jle    4000300d <fmt_fp+0x88d>
    400033cd:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
    400033d2:	e9 20 fb ff ff       	jmp    40002ef7 <fmt_fp+0x777>
    400033d7:	c6 44 24 3c 01       	movb   $0x1,0x3c(%rsp)
    400033dc:	41 bc 06 00 00 00    	mov    $0x6,%r12d
    400033e2:	e9 7d f4 ff ff       	jmp    40002864 <fmt_fp+0xe4>
    400033e7:	48 63 74 24 20       	movslq 0x20(%rsp),%rsi
    400033ec:	48 8b 7c 24 30       	mov    0x30(%rsp),%rdi
    400033f1:	4c 89 ea             	mov    %r13,%rdx
    400033f4:	e8 37 25 00 00       	call   40005930 <__fwritex>
    400033f9:	41 f6 45 00 20       	testb  $0x20,0x0(%r13)
    400033fe:	0f 85 c4 fa ff ff    	jne    40002ec8 <fmt_fp+0x748>
    40003404:	4c 89 ea             	mov    %r13,%rdx
    40003407:	be 03 00 00 00       	mov    $0x3,%esi
    4000340c:	48 89 ef             	mov    %rbp,%rdi
    4000340f:	e8 1c 25 00 00       	call   40005930 <__fwritex>
    40003414:	e9 af fa ff ff       	jmp    40002ec8 <fmt_fp+0x748>
    40003419:	89 c8                	mov    %ecx,%eax
    4000341b:	45 89 eb             	mov    %r13d,%r11d
    4000341e:	49 89 fc             	mov    %rdi,%r12
    40003421:	83 e0 20             	and    $0x20,%eax
    40003424:	e9 36 fe ff ff       	jmp    4000325f <fmt_fp+0xadf>
    40003429:	45 85 e4             	test   %r12d,%r12d
    4000342c:	b8 01 00 00 00       	mov    $0x1,%eax
    40003431:	44 0f 44 e0          	cmove  %eax,%r12d
    40003435:	41 39 fc             	cmp    %edi,%r12d
    40003438:	0f 8e 30 04 00 00    	jle    4000386e <fmt_fp+0x10ee>
    4000343e:	83 ff fc             	cmp    $0xfffffffc,%edi
    40003441:	0f 8c 27 04 00 00    	jl     4000386e <fmt_fp+0x10ee>
    40003447:	83 6c 24 18 01       	subl   $0x1,0x18(%rsp)
    4000344c:	8d 47 01             	lea    0x1(%rdi),%eax
    4000344f:	41 29 c4             	sub    %eax,%r12d
    40003452:	f6 44 24 14 08       	testb  $0x8,0x14(%rsp)
    40003457:	0f 85 95 fb ff ff    	jne    40002ff2 <fmt_fp+0x872>
    4000345d:	be 09 00 00 00       	mov    $0x9,%esi
    40003462:	48 39 dd             	cmp    %rbx,%rbp
    40003465:	73 3a                	jae    400034a1 <fmt_fp+0xd21>
    40003467:	44 8b 43 fc          	mov    -0x4(%rbx),%r8d
    4000346b:	45 85 c0             	test   %r8d,%r8d
    4000346e:	74 31                	je     400034a1 <fmt_fp+0xd21>
    40003470:	41 69 c0 cd cc cc cc 	imul   $0xcccccccd,%r8d,%eax
    40003477:	d1 c8                	ror    %eax
    40003479:	3d 99 99 99 19       	cmp    $0x19999999,%eax
    4000347e:	0f 87 04 05 00 00    	ja     40003988 <fmt_fp+0x1208>
    40003484:	31 f6                	xor    %esi,%esi
    40003486:	b9 0a 00 00 00       	mov    $0xa,%ecx
    4000348b:	8d 0c 89             	lea    (%rcx,%rcx,4),%ecx
    4000348e:	44 89 c0             	mov    %r8d,%eax
    40003491:	31 d2                	xor    %edx,%edx
    40003493:	83 c6 01             	add    $0x1,%esi
    40003496:	01 c9                	add    %ecx,%ecx
    40003498:	f7 f1                	div    %ecx
    4000349a:	85 d2                	test   %edx,%edx
    4000349c:	74 ed                	je     4000348b <fmt_fp+0xd0b>
    4000349e:	48 63 f6             	movslq %esi,%rsi
    400034a1:	48 89 d8             	mov    %rbx,%rax
    400034a4:	48 2b 44 24 28       	sub    0x28(%rsp),%rax
    400034a9:	4d 63 e4             	movslq %r12d,%r12
    400034ac:	48 c1 f8 02          	sar    $0x2,%rax
    400034b0:	48 8d 54 c0 f7       	lea    -0x9(%rax,%rax,8),%rdx
    400034b5:	8b 44 24 18          	mov    0x18(%rsp),%eax
    400034b9:	83 c8 20             	or     $0x20,%eax
    400034bc:	83 f8 66             	cmp    $0x66,%eax
    400034bf:	0f 84 fc 03 00 00    	je     400038c1 <fmt_fp+0x1141>
    400034c5:	48 63 c7             	movslq %edi,%rax
    400034c8:	48 01 d0             	add    %rdx,%rax
    400034cb:	48 29 f0             	sub    %rsi,%rax
    400034ce:	ba 00 00 00 00       	mov    $0x0,%edx
    400034d3:	48 0f 48 c2          	cmovs  %rdx,%rax
    400034d7:	4c 39 e0             	cmp    %r12,%rax
    400034da:	4c 0f 4e e0          	cmovle %rax,%r12
    400034de:	e9 0f fb ff ff       	jmp    40002ff2 <fmt_fp+0x872>
    400034e3:	45 85 f6             	test   %r14d,%r14d
    400034e6:	0f 8f e8 f8 ff ff    	jg     40002dd4 <fmt_fp+0x654>
    400034ec:	e9 d7 f9 ff ff       	jmp    40002ec8 <fmt_fp+0x748>
    400034f1:	44 39 df             	cmp    %r11d,%edi
    400034f4:	0f 8f d3 fe ff ff    	jg     400033cd <fmt_fp+0xc4d>
    400034fa:	41 8d 04 3e          	lea    (%r14,%rdi,1),%eax
    400034fe:	85 ff                	test   %edi,%edi
    40003500:	44 0f 4f f0          	cmovg  %eax,%r14d
    40003504:	e9 c8 fb ff ff       	jmp    400030d1 <fmt_fp+0x951>
    40003509:	48 8b 44 24 28       	mov    0x28(%rsp),%rax
    4000350e:	49 89 ee             	mov    %rbp,%r14
    40003511:	48 89 5c 24 08       	mov    %rbx,0x8(%rsp)
    40003516:	49 bf cd cc cc cc cc 	movabs $0xcccccccccccccccd,%r15
    4000351d:	cc cc cc 
    40003520:	48 39 e8             	cmp    %rbp,%rax
    40003523:	4c 0f 46 f0          	cmovbe %rax,%r14
    40003527:	4c 89 f5             	mov    %r14,%rbp
    4000352a:	4c 89 f3             	mov    %r14,%rbx
    4000352d:	4d 89 ee             	mov    %r13,%r14
    40003530:	49 89 ed             	mov    %rbp,%r13
    40003533:	48 89 c5             	mov    %rax,%rbp
    40003536:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    4000353d:	00 00 00 
    40003540:	41 8b 4d 00          	mov    0x0(%r13),%ecx
    40003544:	48 85 c9             	test   %rcx,%rcx
    40003547:	0f 84 29 02 00 00    	je     40003776 <fmt_fp+0xff6>
    4000354d:	48 8d 74 24 69       	lea    0x69(%rsp),%rsi
    40003552:	48 89 f7             	mov    %rsi,%rdi
    40003555:	0f 1f 00             	nopl   (%rax)
    40003558:	48 89 c8             	mov    %rcx,%rax
    4000355b:	48 83 ef 01          	sub    $0x1,%rdi
    4000355f:	49 f7 e7             	mul    %r15
    40003562:	48 89 c8             	mov    %rcx,%rax
    40003565:	48 c1 ea 03          	shr    $0x3,%rdx
    40003569:	4c 8d 04 92          	lea    (%rdx,%rdx,4),%r8
    4000356d:	4d 01 c0             	add    %r8,%r8
    40003570:	4c 29 c0             	sub    %r8,%rax
    40003573:	83 c0 30             	add    $0x30,%eax
    40003576:	88 07                	mov    %al,(%rdi)
    40003578:	48 89 c8             	mov    %rcx,%rax
    4000357b:	48 89 d1             	mov    %rdx,%rcx
    4000357e:	48 83 f8 09          	cmp    $0x9,%rax
    40003582:	77 d4                	ja     40003558 <fmt_fp+0xdd8>
    40003584:	4c 39 eb             	cmp    %r13,%rbx
    40003587:	0f 84 93 01 00 00    	je     40003720 <fmt_fp+0xfa0>
    4000358d:	48 8d 54 24 60       	lea    0x60(%rsp),%rdx
    40003592:	48 39 d7             	cmp    %rdx,%rdi
    40003595:	76 30                	jbe    400035c7 <fmt_fp+0xe47>
    40003597:	48 89 f8             	mov    %rdi,%rax
    4000359a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    400035a0:	48 83 e8 01          	sub    $0x1,%rax
    400035a4:	c6 00 30             	movb   $0x30,(%rax)
    400035a7:	48 39 d0             	cmp    %rdx,%rax
    400035aa:	77 f4                	ja     400035a0 <fmt_fp+0xe20>
    400035ac:	48 89 d0             	mov    %rdx,%rax
    400035af:	48 8d 4f ff          	lea    -0x1(%rdi),%rcx
    400035b3:	48 29 f8             	sub    %rdi,%rax
    400035b6:	48 39 ca             	cmp    %rcx,%rdx
    400035b9:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
    400035c0:	48 0f 47 c2          	cmova  %rdx,%rax
    400035c4:	48 01 c7             	add    %rax,%rdi
    400035c7:	48 29 fe             	sub    %rdi,%rsi
    400035ca:	41 f6 06 20          	testb  $0x20,(%r14)
    400035ce:	0f 84 74 01 00 00    	je     40003748 <fmt_fp+0xfc8>
    400035d4:	49 83 c5 04          	add    $0x4,%r13
    400035d8:	49 39 ed             	cmp    %rbp,%r13
    400035db:	0f 86 5f ff ff ff    	jbe    40003540 <fmt_fp+0xdc0>
    400035e1:	48 8b 7c 24 28       	mov    0x28(%rsp),%rdi
    400035e6:	4d 89 f5             	mov    %r14,%r13
    400035e9:	49 89 de             	mov    %rbx,%r14
    400035ec:	48 8b 5c 24 08       	mov    0x8(%rsp),%rbx
    400035f1:	48 89 f8             	mov    %rdi,%rax
    400035f4:	48 83 c7 01          	add    $0x1,%rdi
    400035f8:	4c 29 f0             	sub    %r14,%rax
    400035fb:	48 c1 e8 02          	shr    $0x2,%rax
    400035ff:	48 8d 0c 85 04 00 00 	lea    0x4(,%rax,4),%rcx
    40003606:	00 
    40003607:	49 8d 46 01          	lea    0x1(%r14),%rax
    4000360b:	48 39 c7             	cmp    %rax,%rdi
    4000360e:	b8 04 00 00 00       	mov    $0x4,%eax
    40003613:	48 0f 42 c8          	cmovb  %rax,%rcx
    40003617:	4c 01 f1             	add    %r14,%rcx
    4000361a:	45 85 e4             	test   %r12d,%r12d
    4000361d:	0f 85 09 03 00 00    	jne    4000392c <fmt_fp+0x11ac>
    40003623:	f6 44 24 14 08       	testb  $0x8,0x14(%rsp)
    40003628:	0f 84 ab fc ff ff    	je     400032d9 <fmt_fp+0xb59>
    4000362e:	41 f6 45 00 20       	testb  $0x20,0x0(%r13)
    40003633:	0f 85 a0 fc ff ff    	jne    400032d9 <fmt_fp+0xb59>
    40003639:	4c 89 ea             	mov    %r13,%rdx
    4000363c:	be 01 00 00 00       	mov    $0x1,%esi
    40003641:	bf 6b 6c 00 40       	mov    $0x40006c6b,%edi
    40003646:	48 89 4c 24 08       	mov    %rcx,0x8(%rsp)
    4000364b:	e8 e0 22 00 00       	call   40005930 <__fwritex>
    40003650:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
    40003655:	45 85 e4             	test   %r12d,%r12d
    40003658:	0f 8e 98 02 00 00    	jle    400038f6 <fmt_fp+0x1176>
    4000365e:	48 39 cb             	cmp    %rcx,%rbx
    40003661:	0f 86 8f 02 00 00    	jbe    400038f6 <fmt_fp+0x1176>
    40003667:	4c 8d 7c 24 60       	lea    0x60(%rsp),%r15
    4000366c:	4c 8d 4c 24 69       	lea    0x69(%rsp),%r9
    40003671:	49 89 ce             	mov    %rcx,%r14
    40003674:	48 bd cd cc cc cc cc 	movabs $0xcccccccccccccccd,%rbp
    4000367b:	cc cc cc 
    4000367e:	66 90                	xchg   %ax,%ax
    40003680:	41 8b 36             	mov    (%r14),%esi
    40003683:	4c 89 cf             	mov    %r9,%rdi
    40003686:	48 85 f6             	test   %rsi,%rsi
    40003689:	74 36                	je     400036c1 <fmt_fp+0xf41>
    4000368b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40003690:	48 89 f0             	mov    %rsi,%rax
    40003693:	48 83 ef 01          	sub    $0x1,%rdi
    40003697:	48 f7 e5             	mul    %rbp
    4000369a:	48 89 f0             	mov    %rsi,%rax
    4000369d:	48 c1 ea 03          	shr    $0x3,%rdx
    400036a1:	48 8d 0c 92          	lea    (%rdx,%rdx,4),%rcx
    400036a5:	48 01 c9             	add    %rcx,%rcx
    400036a8:	48 29 c8             	sub    %rcx,%rax
    400036ab:	83 c0 30             	add    $0x30,%eax
    400036ae:	88 07                	mov    %al,(%rdi)
    400036b0:	48 89 f0             	mov    %rsi,%rax
    400036b3:	48 89 d6             	mov    %rdx,%rsi
    400036b6:	48 83 f8 09          	cmp    $0x9,%rax
    400036ba:	77 d4                	ja     40003690 <fmt_fp+0xf10>
    400036bc:	4c 39 ff             	cmp    %r15,%rdi
    400036bf:	76 2e                	jbe    400036ef <fmt_fp+0xf6f>
    400036c1:	48 89 f8             	mov    %rdi,%rax
    400036c4:	0f 1f 40 00          	nopl   0x0(%rax)
    400036c8:	48 83 e8 01          	sub    $0x1,%rax
    400036cc:	c6 00 30             	movb   $0x30,(%rax)
    400036cf:	4c 39 f8             	cmp    %r15,%rax
    400036d2:	77 f4                	ja     400036c8 <fmt_fp+0xf48>
    400036d4:	48 8d 57 ff          	lea    -0x1(%rdi),%rdx
    400036d8:	4c 89 f8             	mov    %r15,%rax
    400036db:	48 29 f8             	sub    %rdi,%rax
    400036de:	49 39 d7             	cmp    %rdx,%r15
    400036e1:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
    400036e8:	48 0f 47 c2          	cmova  %rdx,%rax
    400036ec:	48 01 c7             	add    %rax,%rdi
    400036ef:	41 f6 45 00 20       	testb  $0x20,0x0(%r13)
    400036f4:	0f 84 d6 01 00 00    	je     400038d0 <fmt_fp+0x1150>
    400036fa:	49 83 c6 04          	add    $0x4,%r14
    400036fe:	41 8d 44 24 f7       	lea    -0x9(%r12),%eax
    40003703:	49 39 de             	cmp    %rbx,%r14
    40003706:	0f 83 ee 01 00 00    	jae    400038fa <fmt_fp+0x117a>
    4000370c:	85 c0                	test   %eax,%eax
    4000370e:	0f 8e e6 01 00 00    	jle    400038fa <fmt_fp+0x117a>
    40003714:	41 89 c4             	mov    %eax,%r12d
    40003717:	e9 64 ff ff ff       	jmp    40003680 <fmt_fp+0xf00>
    4000371c:	0f 1f 40 00          	nopl   0x0(%rax)
    40003720:	48 39 f7             	cmp    %rsi,%rdi
    40003723:	0f 85 9e fe ff ff    	jne    400035c7 <fmt_fp+0xe47>
    40003729:	c6 44 24 68 30       	movb   $0x30,0x68(%rsp)
    4000372e:	be 01 00 00 00       	mov    $0x1,%esi
    40003733:	48 8d 7c 24 68       	lea    0x68(%rsp),%rdi
    40003738:	41 f6 06 20          	testb  $0x20,(%r14)
    4000373c:	0f 85 92 fe ff ff    	jne    400035d4 <fmt_fp+0xe54>
    40003742:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40003748:	4c 89 f2             	mov    %r14,%rdx
    4000374b:	e8 e0 21 00 00       	call   40005930 <__fwritex>
    40003750:	e9 7f fe ff ff       	jmp    400035d4 <fmt_fp+0xe54>
    40003755:	48 63 74 24 20       	movslq 0x20(%rsp),%rsi
    4000375a:	48 8b 7c 24 30       	mov    0x30(%rsp),%rdi
    4000375f:	4c 89 ea             	mov    %r13,%rdx
    40003762:	44 89 44 24 08       	mov    %r8d,0x8(%rsp)
    40003767:	e8 c4 21 00 00       	call   40005930 <__fwritex>
    4000376c:	44 8b 44 24 08       	mov    0x8(%rsp),%r8d
    40003771:	e9 bf f9 ff ff       	jmp    40003135 <fmt_fp+0x9b5>
    40003776:	4c 39 eb             	cmp    %r13,%rbx
    40003779:	74 ae                	je     40003729 <fmt_fp+0xfa9>
    4000377b:	48 8d 74 24 69       	lea    0x69(%rsp),%rsi
    40003780:	48 8d 54 24 60       	lea    0x60(%rsp),%rdx
    40003785:	48 89 f7             	mov    %rsi,%rdi
    40003788:	e9 0a fe ff ff       	jmp    40003597 <fmt_fp+0xe17>
    4000378d:	48 8b 7c 24 40       	mov    0x40(%rsp),%rdi
    40003792:	48 8d 74 24 60       	lea    0x60(%rsp),%rsi
    40003797:	4c 89 ea             	mov    %r13,%rdx
    4000379a:	48 29 fe             	sub    %rdi,%rsi
    4000379d:	e8 8e 21 00 00       	call   40005930 <__fwritex>
    400037a2:	e9 32 fb ff ff       	jmp    400032d9 <fmt_fp+0xb59>
    400037a7:	44 01 d0             	add    %r10d,%eax
    400037aa:	89 06                	mov    %eax,(%rsi)
    400037ac:	3d ff c9 9a 3b       	cmp    $0x3b9ac9ff,%eax
    400037b1:	0f 86 c8 01 00 00    	jbe    4000397f <fmt_fp+0x11ff>
    400037b7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    400037be:	00 00 
    400037c0:	48 89 f2             	mov    %rsi,%rdx
    400037c3:	48 83 ee 04          	sub    $0x4,%rsi
    400037c7:	c7 46 04 00 00 00 00 	movl   $0x0,0x4(%rsi)
    400037ce:	48 39 ee             	cmp    %rbp,%rsi
    400037d1:	73 0b                	jae    400037de <fmt_fp+0x105e>
    400037d3:	c7 45 fc 00 00 00 00 	movl   $0x0,-0x4(%rbp)
    400037da:	48 83 ed 04          	sub    $0x4,%rbp
    400037de:	8b 06                	mov    (%rsi),%eax
    400037e0:	83 c0 01             	add    $0x1,%eax
    400037e3:	89 06                	mov    %eax,(%rsi)
    400037e5:	3d ff c9 9a 3b       	cmp    $0x3b9ac9ff,%eax
    400037ea:	77 d4                	ja     400037c0 <fmt_fp+0x1040>
    400037ec:	48 8b 44 24 28       	mov    0x28(%rsp),%rax
    400037f1:	8b 4d 00             	mov    0x0(%rbp),%ecx
    400037f4:	48 29 e8             	sub    %rbp,%rax
    400037f7:	48 c1 f8 02          	sar    $0x2,%rax
    400037fb:	8d 3c c0             	lea    (%rax,%rax,8),%edi
    400037fe:	83 f9 09             	cmp    $0x9,%ecx
    40003801:	0f 86 be f7 ff ff    	jbe    40002fc5 <fmt_fp+0x845>
    40003807:	b8 0a 00 00 00       	mov    $0xa,%eax
    4000380c:	0f 1f 40 00          	nopl   0x0(%rax)
    40003810:	8d 04 80             	lea    (%rax,%rax,4),%eax
    40003813:	83 c7 01             	add    $0x1,%edi
    40003816:	01 c0                	add    %eax,%eax
    40003818:	39 c8                	cmp    %ecx,%eax
    4000381a:	76 f4                	jbe    40003810 <fmt_fp+0x1090>
    4000381c:	e9 a4 f7 ff ff       	jmp    40002fc5 <fmt_fp+0x845>
    40003821:	48 8b 7c 24 30       	mov    0x30(%rsp),%rdi
    40003826:	4c 89 ea             	mov    %r13,%rdx
    40003829:	4c 89 d6             	mov    %r10,%rsi
    4000382c:	4c 89 44 24 08       	mov    %r8,0x8(%rsp)
    40003831:	e8 fa 20 00 00       	call   40005930 <__fwritex>
    40003836:	8b 44 24 14          	mov    0x14(%rsp),%eax
    4000383a:	4c 8b 44 24 08       	mov    0x8(%rsp),%r8
    4000383f:	35 00 00 01 00       	xor    $0x10000,%eax
    40003844:	a9 00 20 01 00       	test   $0x12000,%eax
    40003849:	0f 85 67 f5 ff ff    	jne    40002db6 <fmt_fp+0x636>
    4000384f:	45 84 e4             	test   %r12b,%r12b
    40003852:	0f 84 41 f5 ff ff    	je     40002d99 <fmt_fp+0x619>
    40003858:	e9 59 f5 ff ff       	jmp    40002db6 <fmt_fp+0x636>
    4000385d:	db 2d 2d 38 00 00    	fldt   0x382d(%rip)        # 40007090 <states+0x1f0>
    40003863:	d9 05 0f 38 00 00    	flds   0x380f(%rip)        # 40007078 <states+0x1d8>
    40003869:	e9 1f f7 ff ff       	jmp    40002f8d <fmt_fp+0x80d>
    4000386e:	83 6c 24 18 02       	subl   $0x2,0x18(%rsp)
    40003873:	41 83 ec 01          	sub    $0x1,%r12d
    40003877:	e9 d6 fb ff ff       	jmp    40003452 <fmt_fp+0xcd2>
    4000387c:	c6 44 24 5f 30       	movb   $0x30,0x5f(%rsp)
    40003881:	48 8d 7c 24 5f       	lea    0x5f(%rsp),%rdi
    40003886:	e9 aa f3 ff ff       	jmp    40002c35 <fmt_fp+0x4b5>
    4000388b:	81 f9 00 ca 9a 3b    	cmp    $0x3b9aca00,%ecx
    40003891:	75 0f                	jne    400038a2 <fmt_fp+0x1122>
    40003893:	48 39 ee             	cmp    %rbp,%rsi
    40003896:	76 0a                	jbe    400038a2 <fmt_fp+0x1122>
    40003898:	f6 46 fc 01          	testb  $0x1,-0x4(%rsi)
    4000389c:	0f 85 ce f6 ff ff    	jne    40002f70 <fmt_fp+0x7f0>
    400038a2:	d9 05 cc 37 00 00    	flds   0x37cc(%rip)        # 40007074 <states+0x1d4>
    400038a8:	e9 c9 f6 ff ff       	jmp    40002f76 <fmt_fp+0x7f6>
    400038ad:	45 8d 4c 24 02       	lea    0x2(%r12),%r9d
    400038b2:	45 89 ce             	mov    %r9d,%r14d
    400038b5:	41 8d 1c 29          	lea    (%r9,%rbp,1),%ebx
    400038b9:	45 29 c6             	sub    %r8d,%r14d
    400038bc:	e9 72 f4 ff ff       	jmp    40002d33 <fmt_fp+0x5b3>
    400038c1:	48 89 d0             	mov    %rdx,%rax
    400038c4:	e9 02 fc ff ff       	jmp    400034cb <fmt_fp+0xd4b>
    400038c9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    400038d0:	b8 09 00 00 00       	mov    $0x9,%eax
    400038d5:	4c 89 ea             	mov    %r13,%rdx
    400038d8:	4c 89 4c 24 08       	mov    %r9,0x8(%rsp)
    400038dd:	41 39 c4             	cmp    %eax,%r12d
    400038e0:	41 0f 4e c4          	cmovle %r12d,%eax
    400038e4:	48 63 f0             	movslq %eax,%rsi
    400038e7:	e8 44 20 00 00       	call   40005930 <__fwritex>
    400038ec:	4c 8b 4c 24 08       	mov    0x8(%rsp),%r9
    400038f1:	e9 04 fe ff ff       	jmp    400036fa <fmt_fp+0xf7a>
    400038f6:	41 83 c4 09          	add    $0x9,%r12d
    400038fa:	41 83 fc 09          	cmp    $0x9,%r12d
    400038fe:	0f 8e d5 f9 ff ff    	jle    400032d9 <fmt_fp+0xb59>
    40003904:	b9 09 00 00 00       	mov    $0x9,%ecx
    40003909:	44 89 e2             	mov    %r12d,%edx
    4000390c:	be 30 00 00 00       	mov    $0x30,%esi
    40003911:	4c 89 ef             	mov    %r13,%rdi
    40003914:	e8 a7 ed ff ff       	call   400026c0 <pad.part.0>
    40003919:	e9 bb f9 ff ff       	jmp    400032d9 <fmt_fp+0xb59>
    4000391e:	dd d8                	fstp   %st(0)
    40003920:	dd d8                	fstp   %st(0)
    40003922:	dd d8                	fstp   %st(0)
    40003924:	49 89 c0             	mov    %rax,%r8
    40003927:	e9 c8 f3 ff ff       	jmp    40002cf4 <fmt_fp+0x574>
    4000392c:	41 f6 45 00 20       	testb  $0x20,0x0(%r13)
    40003931:	0f 85 1e fd ff ff    	jne    40003655 <fmt_fp+0xed5>
    40003937:	e9 fd fc ff ff       	jmp    40003639 <fmt_fp+0xeb9>
    4000393c:	0f 1f 40 00          	nopl   0x0(%rax)
    40003940:	d9 cb                	fxch   %st(3)
    40003942:	d9 e0                	fchs   
    40003944:	d8 e3                	fsub   %st(3),%st
    40003946:	de c3                	faddp  %st,%st(3)
    40003948:	d9 ca                	fxch   %st(2)
    4000394a:	d9 e0                	fchs   
    4000394c:	d9 ca                	fxch   %st(2)
    4000394e:	e9 86 f2 ff ff       	jmp    40002bd9 <fmt_fp+0x459>
    40003953:	48 8d 46 04          	lea    0x4(%rsi),%rax
    40003957:	48 39 c3             	cmp    %rax,%rbx
    4000395a:	0f 85 27 f6 ff ff    	jne    40002f87 <fmt_fp+0x807>
    40003960:	d9 e8                	fld1   
    40003962:	e9 26 f6 ff ff       	jmp    40002f8d <fmt_fp+0x80d>
    40003967:	48 89 da             	mov    %rbx,%rdx
    4000396a:	e9 56 f6 ff ff       	jmp    40002fc5 <fmt_fp+0x845>
    4000396f:	41 ba 0a 00 00 00    	mov    $0xa,%r10d
    40003975:	b9 0a 00 00 00       	mov    $0xa,%ecx
    4000397a:	e9 71 f1 ff ff       	jmp    40002af0 <fmt_fp+0x370>
    4000397f:	48 8d 56 04          	lea    0x4(%rsi),%rdx
    40003983:	e9 64 fe ff ff       	jmp    400037ec <fmt_fp+0x106c>
    40003988:	31 f6                	xor    %esi,%esi
    4000398a:	e9 12 fb ff ff       	jmp    400034a1 <fmt_fp+0xd21>
    4000398f:	90                   	nop

0000000040003990 <printf_core>:
    40003990:	41 57                	push   %r15
    40003992:	31 c0                	xor    %eax,%eax
    40003994:	49 89 f9             	mov    %rdi,%r9
    40003997:	41 56                	push   %r14
    40003999:	41 89 c6             	mov    %eax,%r14d
    4000399c:	41 55                	push   %r13
    4000399e:	45 31 ed             	xor    %r13d,%r13d
    400039a1:	41 54                	push   %r12
    400039a3:	45 89 ea             	mov    %r13d,%r10d
    400039a6:	55                   	push   %rbp
    400039a7:	48 89 f5             	mov    %rsi,%rbp
    400039aa:	53                   	push   %rbx
    400039ab:	48 81 ec 98 00 00 00 	sub    $0x98,%rsp
    400039b2:	48 89 54 24 10       	mov    %rdx,0x10(%rsp)
    400039b7:	48 89 4c 24 18       	mov    %rcx,0x18(%rsp)
    400039bc:	4c 89 44 24 20       	mov    %r8,0x20(%rsp)
    400039c1:	c7 44 24 0c 00 00 00 	movl   $0x0,0xc(%rsp)
    400039c8:	00 
    400039c9:	0f b6 45 00          	movzbl 0x0(%rbp),%eax
    400039cd:	45 01 d6             	add    %r10d,%r14d
    400039d0:	84 c0                	test   %al,%al
    400039d2:	0f 84 4e 05 00 00    	je     40003f26 <printf_core+0x596>
    400039d8:	49 89 eb             	mov    %rbp,%r11
    400039db:	eb 10                	jmp    400039ed <printf_core+0x5d>
    400039dd:	0f 1f 00             	nopl   (%rax)
    400039e0:	41 0f b6 43 01       	movzbl 0x1(%r11),%eax
    400039e5:	49 83 c3 01          	add    $0x1,%r11
    400039e9:	84 c0                	test   %al,%al
    400039eb:	74 6b                	je     40003a58 <printf_core+0xc8>
    400039ed:	3c 25                	cmp    $0x25,%al
    400039ef:	75 ef                	jne    400039e0 <printf_core+0x50>
    400039f1:	4c 89 db             	mov    %r11,%rbx
    400039f4:	eb 17                	jmp    40003a0d <printf_core+0x7d>
    400039f6:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    400039fd:	00 00 00 
    40003a00:	48 83 c3 02          	add    $0x2,%rbx
    40003a04:	49 83 c3 01          	add    $0x1,%r11
    40003a08:	80 3b 25             	cmpb   $0x25,(%rbx)
    40003a0b:	75 06                	jne    40003a13 <printf_core+0x83>
    40003a0d:	80 7b 01 25          	cmpb   $0x25,0x1(%rbx)
    40003a11:	74 ed                	je     40003a00 <printf_core+0x70>
    40003a13:	41 bd ff ff ff 7f    	mov    $0x7fffffff,%r13d
    40003a19:	49 29 eb             	sub    %rbp,%r11
    40003a1c:	45 29 f5             	sub    %r14d,%r13d
    40003a1f:	49 63 c5             	movslq %r13d,%rax
    40003a22:	49 39 c3             	cmp    %rax,%r11
    40003a25:	0f 8f 78 04 00 00    	jg     40003ea3 <printf_core+0x513>
    40003a2b:	45 89 da             	mov    %r11d,%r10d
    40003a2e:	4d 85 c9             	test   %r9,%r9
    40003a31:	74 0a                	je     40003a3d <printf_core+0xad>
    40003a33:	41 f6 01 20          	testb  $0x20,(%r9)
    40003a37:	0f 84 5b 02 00 00    	je     40003c98 <printf_core+0x308>
    40003a3d:	45 85 db             	test   %r11d,%r11d
    40003a40:	74 26                	je     40003a68 <printf_core+0xd8>
    40003a42:	45 39 d5             	cmp    %r10d,%r13d
    40003a45:	0f 8c 58 04 00 00    	jl     40003ea3 <printf_core+0x513>
    40003a4b:	48 89 dd             	mov    %rbx,%rbp
    40003a4e:	e9 76 ff ff ff       	jmp    400039c9 <printf_core+0x39>
    40003a53:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40003a58:	4c 89 db             	mov    %r11,%rbx
    40003a5b:	3c 25                	cmp    $0x25,%al
    40003a5d:	74 92                	je     400039f1 <printf_core+0x61>
    40003a5f:	eb b2                	jmp    40003a13 <printf_core+0x83>
    40003a61:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40003a68:	0f be 4b 01          	movsbl 0x1(%rbx),%ecx
    40003a6c:	8d 51 d0             	lea    -0x30(%rcx),%edx
    40003a6f:	89 ce                	mov    %ecx,%esi
    40003a71:	83 fa 09             	cmp    $0x9,%edx
    40003a74:	77 0a                	ja     40003a80 <printf_core+0xf0>
    40003a76:	80 7b 02 24          	cmpb   $0x24,0x2(%rbx)
    40003a7a:	0f 84 18 03 00 00    	je     40003d98 <printf_core+0x408>
    40003a80:	48 83 c3 01          	add    $0x1,%rbx
    40003a84:	ba ff ff ff ff       	mov    $0xffffffff,%edx
    40003a89:	8d 79 e0             	lea    -0x20(%rcx),%edi
    40003a8c:	45 31 e4             	xor    %r12d,%r12d
    40003a8f:	b8 89 28 01 00       	mov    $0x12889,%eax
    40003a94:	83 ff 1f             	cmp    $0x1f,%edi
    40003a97:	76 22                	jbe    40003abb <printf_core+0x12b>
    40003a99:	eb 28                	jmp    40003ac3 <printf_core+0x133>
    40003a9b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40003aa0:	bf 01 00 00 00       	mov    $0x1,%edi
    40003aa5:	48 83 c3 01          	add    $0x1,%rbx
    40003aa9:	d3 e7                	shl    %cl,%edi
    40003aab:	0f be 0b             	movsbl (%rbx),%ecx
    40003aae:	41 09 fc             	or     %edi,%r12d
    40003ab1:	8d 79 e0             	lea    -0x20(%rcx),%edi
    40003ab4:	89 ce                	mov    %ecx,%esi
    40003ab6:	83 ff 1f             	cmp    $0x1f,%edi
    40003ab9:	77 08                	ja     40003ac3 <printf_core+0x133>
    40003abb:	83 e9 20             	sub    $0x20,%ecx
    40003abe:	0f a3 c8             	bt     %ecx,%eax
    40003ac1:	72 dd                	jb     40003aa0 <printf_core+0x110>
    40003ac3:	40 80 fe 2a          	cmp    $0x2a,%sil
    40003ac7:	0f 84 f3 00 00 00    	je     40003bc0 <printf_core+0x230>
    40003acd:	0f be 03             	movsbl (%rbx),%eax
    40003ad0:	45 89 df             	mov    %r11d,%r15d
    40003ad3:	89 c1                	mov    %eax,%ecx
    40003ad5:	83 e8 30             	sub    $0x30,%eax
    40003ad8:	89 ce                	mov    %ecx,%esi
    40003ada:	83 f8 09             	cmp    $0x9,%eax
    40003add:	76 2d                	jbe    40003b0c <printf_core+0x17c>
    40003adf:	eb 5f                	jmp    40003b40 <printf_core+0x1b0>
    40003ae1:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40003ae8:	41 6b ff f6          	imul   $0xfffffff6,%r15d,%edi
    40003aec:	83 e9 30             	sub    $0x30,%ecx
    40003aef:	44 8d 87 ff ff ff 7f 	lea    0x7fffffff(%rdi),%r8d
    40003af6:	44 39 c1             	cmp    %r8d,%ecx
    40003af9:	7f 27                	jg     40003b22 <printf_core+0x192>
    40003afb:	29 f9                	sub    %edi,%ecx
    40003afd:	41 89 cf             	mov    %ecx,%r15d
    40003b00:	83 f8 09             	cmp    $0x9,%eax
    40003b03:	77 33                	ja     40003b38 <printf_core+0x1a8>
    40003b05:	0f 1f 00             	nopl   (%rax)
    40003b08:	40 0f be ce          	movsbl %sil,%ecx
    40003b0c:	0f be 43 01          	movsbl 0x1(%rbx),%eax
    40003b10:	48 83 c3 01          	add    $0x1,%rbx
    40003b14:	89 c6                	mov    %eax,%esi
    40003b16:	83 e8 30             	sub    $0x30,%eax
    40003b19:	41 81 ff cc cc cc 0c 	cmp    $0xccccccc,%r15d
    40003b20:	76 c6                	jbe    40003ae8 <printf_core+0x158>
    40003b22:	83 f8 09             	cmp    $0x9,%eax
    40003b25:	0f 87 78 03 00 00    	ja     40003ea3 <printf_core+0x513>
    40003b2b:	41 bf ff ff ff ff    	mov    $0xffffffff,%r15d
    40003b31:	eb d5                	jmp    40003b08 <printf_core+0x178>
    40003b33:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40003b38:	85 c9                	test   %ecx,%ecx
    40003b3a:	0f 88 63 03 00 00    	js     40003ea3 <printf_core+0x513>
    40003b40:	40 80 fe 2e          	cmp    $0x2e,%sil
    40003b44:	0f 85 e6 00 00 00    	jne    40003c30 <printf_core+0x2a0>
    40003b4a:	0f b6 73 01          	movzbl 0x1(%rbx),%esi
    40003b4e:	40 80 fe 2a          	cmp    $0x2a,%sil
    40003b52:	0f 85 88 02 00 00    	jne    40003de0 <printf_core+0x450>
    40003b58:	48 0f be 43 02       	movsbq 0x2(%rbx),%rax
    40003b5d:	8d 48 d0             	lea    -0x30(%rax),%ecx
    40003b60:	48 89 c6             	mov    %rax,%rsi
    40003b63:	83 f9 09             	cmp    $0x9,%ecx
    40003b66:	77 0a                	ja     40003b72 <printf_core+0x1e2>
    40003b68:	80 7b 03 24          	cmpb   $0x24,0x3(%rbx)
    40003b6c:	0f 84 75 03 00 00    	je     40003ee7 <printf_core+0x557>
    40003b72:	8b 7c 24 0c          	mov    0xc(%rsp),%edi
    40003b76:	85 ff                	test   %edi,%edi
    40003b78:	0f 85 f5 00 00 00    	jne    40003c73 <printf_core+0x2e3>
    40003b7e:	4d 85 c9             	test   %r9,%r9
    40003b81:	0f 84 91 03 00 00    	je     40003f18 <printf_core+0x588>
    40003b87:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
    40003b8c:	8b 07                	mov    (%rdi),%eax
    40003b8e:	83 f8 2f             	cmp    $0x2f,%eax
    40003b91:	0f 87 bc 02 00 00    	ja     40003e53 <printf_core+0x4c3>
    40003b97:	89 c1                	mov    %eax,%ecx
    40003b99:	83 c0 08             	add    $0x8,%eax
    40003b9c:	48 03 4f 10          	add    0x10(%rdi),%rcx
    40003ba0:	89 07                	mov    %eax,(%rdi)
    40003ba2:	8b 01                	mov    (%rcx),%eax
    40003ba4:	89 04 24             	mov    %eax,(%rsp)
    40003ba7:	f7 d0                	not    %eax
    40003ba9:	c1 e8 1f             	shr    $0x1f,%eax
    40003bac:	89 c1                	mov    %eax,%ecx
    40003bae:	0f be 43 02          	movsbl 0x2(%rbx),%eax
    40003bb2:	48 83 c3 02          	add    $0x2,%rbx
    40003bb6:	e9 83 00 00 00       	jmp    40003c3e <printf_core+0x2ae>
    40003bbb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40003bc0:	48 0f be 43 01       	movsbq 0x1(%rbx),%rax
    40003bc5:	48 89 c6             	mov    %rax,%rsi
    40003bc8:	83 e8 30             	sub    $0x30,%eax
    40003bcb:	83 f8 09             	cmp    $0x9,%eax
    40003bce:	0f 86 8c 01 00 00    	jbe    40003d60 <printf_core+0x3d0>
    40003bd4:	44 8b 44 24 0c       	mov    0xc(%rsp),%r8d
    40003bd9:	45 85 c0             	test   %r8d,%r8d
    40003bdc:	0f 85 91 00 00 00    	jne    40003c73 <printf_core+0x2e3>
    40003be2:	48 83 c3 01          	add    $0x1,%rbx
    40003be6:	45 89 df             	mov    %r11d,%r15d
    40003be9:	4d 85 c9             	test   %r9,%r9
    40003bec:	0f 84 4e ff ff ff    	je     40003b40 <printf_core+0x1b0>
    40003bf2:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
    40003bf7:	8b 07                	mov    (%rdi),%eax
    40003bf9:	83 f8 2f             	cmp    $0x2f,%eax
    40003bfc:	0f 87 c4 01 00 00    	ja     40003dc6 <printf_core+0x436>
    40003c02:	89 c1                	mov    %eax,%ecx
    40003c04:	83 c0 08             	add    $0x8,%eax
    40003c07:	48 03 4f 10          	add    0x10(%rdi),%rcx
    40003c0b:	89 07                	mov    %eax,(%rdi)
    40003c0d:	44 8b 39             	mov    (%rcx),%r15d
    40003c10:	0f b6 33             	movzbl (%rbx),%esi
    40003c13:	45 85 ff             	test   %r15d,%r15d
    40003c16:	0f 89 24 ff ff ff    	jns    40003b40 <printf_core+0x1b0>
    40003c1c:	41 81 cc 00 20 00 00 	or     $0x2000,%r12d
    40003c23:	41 f7 df             	neg    %r15d
    40003c26:	40 80 fe 2e          	cmp    $0x2e,%sil
    40003c2a:	0f 84 1a ff ff ff    	je     40003b4a <printf_core+0x1ba>
    40003c30:	c7 04 24 ff ff ff ff 	movl   $0xffffffff,(%rsp)
    40003c37:	44 89 d9             	mov    %r11d,%ecx
    40003c3a:	40 0f be c6          	movsbl %sil,%eax
    40003c3e:	45 31 c0             	xor    %r8d,%r8d
    40003c41:	eb 28                	jmp    40003c6b <printf_core+0x2db>
    40003c43:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40003c48:	44 89 c7             	mov    %r8d,%edi
    40003c4b:	48 98                	cltq   
    40003c4d:	48 83 c3 01          	add    $0x1,%rbx
    40003c51:	48 6b f7 3a          	imul   $0x3a,%rdi,%rsi
    40003c55:	0f b6 b4 30 a0 6e 00 	movzbl 0x40006ea0(%rax,%rsi,1),%esi
    40003c5c:	40 
    40003c5d:	8d 46 ff             	lea    -0x1(%rsi),%eax
    40003c60:	83 f8 07             	cmp    $0x7,%eax
    40003c63:	77 6b                	ja     40003cd0 <printf_core+0x340>
    40003c65:	0f be 03             	movsbl (%rbx),%eax
    40003c68:	41 89 f0             	mov    %esi,%r8d
    40003c6b:	83 e8 41             	sub    $0x41,%eax
    40003c6e:	83 f8 39             	cmp    $0x39,%eax
    40003c71:	76 d5                	jbe    40003c48 <printf_core+0x2b8>
    40003c73:	e8 c8 13 00 00       	call   40005040 <__errno_location>
    40003c78:	c7 00 16 00 00 00    	movl   $0x16,(%rax)
    40003c7e:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
    40003c83:	48 81 c4 98 00 00 00 	add    $0x98,%rsp
    40003c8a:	5b                   	pop    %rbx
    40003c8b:	5d                   	pop    %rbp
    40003c8c:	41 5c                	pop    %r12
    40003c8e:	41 5d                	pop    %r13
    40003c90:	41 5e                	pop    %r14
    40003c92:	41 5f                	pop    %r15
    40003c94:	c3                   	ret    
    40003c95:	0f 1f 00             	nopl   (%rax)
    40003c98:	49 63 f3             	movslq %r11d,%rsi
    40003c9b:	4c 89 ca             	mov    %r9,%rdx
    40003c9e:	48 89 ef             	mov    %rbp,%rdi
    40003ca1:	44 89 5c 24 30       	mov    %r11d,0x30(%rsp)
    40003ca6:	4c 89 5c 24 28       	mov    %r11,0x28(%rsp)
    40003cab:	4c 89 0c 24          	mov    %r9,(%rsp)
    40003caf:	e8 7c 1c 00 00       	call   40005930 <__fwritex>
    40003cb4:	44 8b 54 24 30       	mov    0x30(%rsp),%r10d
    40003cb9:	4c 8b 5c 24 28       	mov    0x28(%rsp),%r11
    40003cbe:	4c 8b 0c 24          	mov    (%rsp),%r9
    40003cc2:	e9 76 fd ff ff       	jmp    40003a3d <printf_core+0xad>
    40003cc7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    40003cce:	00 00 
    40003cd0:	48 89 f8             	mov    %rdi,%rax
    40003cd3:	85 f6                	test   %esi,%esi
    40003cd5:	74 9c                	je     40003c73 <printf_core+0x2e3>
    40003cd7:	83 fe 1b             	cmp    $0x1b,%esi
    40003cda:	0f 84 cf 00 00 00    	je     40003daf <printf_core+0x41f>
    40003ce0:	83 fa ff             	cmp    $0xffffffff,%edx
    40003ce3:	0f 84 cf 01 00 00    	je     40003eb8 <printf_core+0x528>
    40003ce9:	48 63 d2             	movslq %edx,%rdx
    40003cec:	4d 85 c9             	test   %r9,%r9
    40003cef:	0f 84 74 01 00 00    	je     40003e69 <printf_core+0x4d9>
    40003cf5:	48 8b 7c 24 18       	mov    0x18(%rsp),%rdi
    40003cfa:	48 c1 e2 04          	shl    $0x4,%rdx
    40003cfe:	66 0f 6f 04 17       	movdqa (%rdi,%rdx,1),%xmm0
    40003d03:	0f 29 44 24 60       	movaps %xmm0,0x60(%rsp)
    40003d08:	41 f6 01 20          	testb  $0x20,(%r9)
    40003d0c:	0f 85 9c 01 00 00    	jne    40003eae <printf_core+0x51e>
    40003d12:	0f be 53 ff          	movsbl -0x1(%rbx),%edx
    40003d16:	89 d6                	mov    %edx,%esi
    40003d18:	45 85 c0             	test   %r8d,%r8d
    40003d1b:	74 13                	je     40003d30 <printf_core+0x3a0>
    40003d1d:	89 d7                	mov    %edx,%edi
    40003d1f:	83 e6 df             	and    $0xffffffdf,%esi
    40003d22:	83 e7 0f             	and    $0xf,%edi
    40003d25:	40 0f be f6          	movsbl %sil,%esi
    40003d29:	40 80 ff 03          	cmp    $0x3,%dil
    40003d2d:	0f 44 d6             	cmove  %esi,%edx
    40003d30:	44 89 e6             	mov    %r12d,%esi
    40003d33:	81 e6 ff ff fe ff    	and    $0xfffeffff,%esi
    40003d39:	41 f7 c4 00 20 00 00 	test   $0x2000,%r12d
    40003d40:	44 0f 45 e6          	cmovne %esi,%r12d
    40003d44:	8d 72 bf             	lea    -0x41(%rdx),%esi
    40003d47:	83 fe 37             	cmp    $0x37,%esi
    40003d4a:	0f 87 38 02 00 00    	ja     40003f88 <printf_core+0x5f8>
    40003d50:	ff 24 f5 80 6c 00 40 	jmp    *0x40006c80(,%rsi,8)
    40003d57:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    40003d5e:	00 00 
    40003d60:	80 7b 02 24          	cmpb   $0x24,0x2(%rbx)
    40003d64:	0f 85 6a fe ff ff    	jne    40003bd4 <printf_core+0x244>
    40003d6a:	48 8d 43 03          	lea    0x3(%rbx),%rax
    40003d6e:	4d 85 c9             	test   %r9,%r9
    40003d71:	0f 84 12 09 00 00    	je     40004689 <printf_core+0xcf9>
    40003d77:	48 8b 7c 24 18       	mov    0x18(%rsp),%rdi
    40003d7c:	48 c1 e6 04          	shl    $0x4,%rsi
    40003d80:	c7 44 24 0c 01 00 00 	movl   $0x1,0xc(%rsp)
    40003d87:	00 
    40003d88:	48 89 c3             	mov    %rax,%rbx
    40003d8b:	44 8b bc 37 00 fd ff 	mov    -0x300(%rdi,%rsi,1),%r15d
    40003d92:	ff 
    40003d93:	e9 78 fe ff ff       	jmp    40003c10 <printf_core+0x280>
    40003d98:	0f be 4b 03          	movsbl 0x3(%rbx),%ecx
    40003d9c:	c7 44 24 0c 01 00 00 	movl   $0x1,0xc(%rsp)
    40003da3:	00 
    40003da4:	48 83 c3 03          	add    $0x3,%rbx
    40003da8:	89 ce                	mov    %ecx,%esi
    40003daa:	e9 da fc ff ff       	jmp    40003a89 <printf_core+0xf9>
    40003daf:	83 fa ff             	cmp    $0xffffffff,%edx
    40003db2:	0f 85 bb fe ff ff    	jne    40003c73 <printf_core+0x2e3>
    40003db8:	4d 85 c9             	test   %r9,%r9
    40003dbb:	0f 85 47 ff ff ff    	jne    40003d08 <printf_core+0x378>
    40003dc1:	e9 85 fc ff ff       	jmp    40003a4b <printf_core+0xbb>
    40003dc6:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
    40003dcb:	48 8b 4f 08          	mov    0x8(%rdi),%rcx
    40003dcf:	48 8d 41 08          	lea    0x8(%rcx),%rax
    40003dd3:	48 89 47 08          	mov    %rax,0x8(%rdi)
    40003dd7:	e9 31 fe ff ff       	jmp    40003c0d <printf_core+0x27d>
    40003ddc:	0f 1f 40 00          	nopl   0x0(%rax)
    40003de0:	40 0f be c6          	movsbl %sil,%eax
    40003de4:	48 83 c3 01          	add    $0x1,%rbx
    40003de8:	8d 48 d0             	lea    -0x30(%rax),%ecx
    40003deb:	83 f9 09             	cmp    $0x9,%ecx
    40003dee:	0f 87 e8 08 00 00    	ja     400046dc <printf_core+0xd4c>
    40003df4:	44 89 d9             	mov    %r11d,%ecx
    40003df7:	bf ff ff ff ff       	mov    $0xffffffff,%edi
    40003dfc:	eb 2b                	jmp    40003e29 <printf_core+0x499>
    40003dfe:	66 90                	xchg   %ax,%ax
    40003e00:	6b c9 f6             	imul   $0xfffffff6,%ecx,%ecx
    40003e03:	83 e8 30             	sub    $0x30,%eax
    40003e06:	89 c6                	mov    %eax,%esi
    40003e08:	29 ce                	sub    %ecx,%esi
    40003e0a:	44 8d 81 ff ff ff 7f 	lea    0x7fffffff(%rcx),%r8d
    40003e11:	44 39 c0             	cmp    %r8d,%eax
    40003e14:	89 f1                	mov    %esi,%ecx
    40003e16:	0f 4f cf             	cmovg  %edi,%ecx
    40003e19:	0f be 43 01          	movsbl 0x1(%rbx),%eax
    40003e1d:	48 83 c3 01          	add    $0x1,%rbx
    40003e21:	8d 70 d0             	lea    -0x30(%rax),%esi
    40003e24:	83 fe 09             	cmp    $0x9,%esi
    40003e27:	77 1d                	ja     40003e46 <printf_core+0x4b6>
    40003e29:	81 f9 cc cc cc 0c    	cmp    $0xccccccc,%ecx
    40003e2f:	76 cf                	jbe    40003e00 <printf_core+0x470>
    40003e31:	0f be 43 01          	movsbl 0x1(%rbx),%eax
    40003e35:	48 83 c3 01          	add    $0x1,%rbx
    40003e39:	b9 ff ff ff ff       	mov    $0xffffffff,%ecx
    40003e3e:	8d 70 d0             	lea    -0x30(%rax),%esi
    40003e41:	83 fe 09             	cmp    $0x9,%esi
    40003e44:	76 e3                	jbe    40003e29 <printf_core+0x499>
    40003e46:	89 0c 24             	mov    %ecx,(%rsp)
    40003e49:	b9 01 00 00 00       	mov    $0x1,%ecx
    40003e4e:	e9 eb fd ff ff       	jmp    40003c3e <printf_core+0x2ae>
    40003e53:	48 8b 7c 24 10       	mov    0x10(%rsp),%rdi
    40003e58:	48 8b 4f 08          	mov    0x8(%rdi),%rcx
    40003e5c:	48 8d 41 08          	lea    0x8(%rcx),%rax
    40003e60:	48 89 47 08          	mov    %rax,0x8(%rdi)
    40003e64:	e9 39 fd ff ff       	jmp    40003ba2 <printf_core+0x212>
    40003e69:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
    40003e6e:	89 34 90             	mov    %esi,(%rax,%rdx,4)
    40003e71:	e9 d5 fb ff ff       	jmp    40003a4b <printf_core+0xbb>
    40003e76:	be ff ff ff 7f       	mov    $0x7fffffff,%esi
    40003e7b:	48 89 ef             	mov    %rbp,%rdi
    40003e7e:	4c 89 4c 24 28       	mov    %r9,0x28(%rsp)
    40003e83:	4c 89 1c 24          	mov    %r11,(%rsp)
    40003e87:	e8 f4 0a 00 00       	call   40004980 <strnlen>
    40003e8c:	4c 8b 1c 24          	mov    (%rsp),%r11
    40003e90:	4c 8b 4c 24 28       	mov    0x28(%rsp),%r9
    40003e95:	48 8d 54 05 00       	lea    0x0(%rbp,%rax,1),%rdx
    40003e9a:	80 3a 00             	cmpb   $0x0,(%rdx)
    40003e9d:	0f 84 49 04 00 00    	je     400042ec <printf_core+0x95c>
    40003ea3:	e8 98 11 00 00       	call   40005040 <__errno_location>
    40003ea8:	c7 00 4b 00 00 00    	movl   $0x4b,(%rax)
    40003eae:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
    40003eb3:	e9 cb fd ff ff       	jmp    40003c83 <printf_core+0x2f3>
    40003eb8:	4d 85 c9             	test   %r9,%r9
    40003ebb:	0f 84 83 08 00 00    	je     40004744 <printf_core+0xdb4>
    40003ec1:	48 8b 54 24 10       	mov    0x10(%rsp),%rdx
    40003ec6:	48 89 7c 24 30       	mov    %rdi,0x30(%rsp)
    40003ecb:	48 8d 7c 24 60       	lea    0x60(%rsp),%rdi
    40003ed0:	89 4c 24 28          	mov    %ecx,0x28(%rsp)
    40003ed4:	e8 07 e6 ff ff       	call   400024e0 <pop_arg>
    40003ed9:	8b 4c 24 28          	mov    0x28(%rsp),%ecx
    40003edd:	48 8b 44 24 30       	mov    0x30(%rsp),%rax
    40003ee2:	e9 21 fe ff ff       	jmp    40003d08 <printf_core+0x378>
    40003ee7:	4d 85 c9             	test   %r9,%r9
    40003eea:	0f 84 c0 07 00 00    	je     400046b0 <printf_core+0xd20>
    40003ef0:	48 8b 44 24 18       	mov    0x18(%rsp),%rax
    40003ef5:	48 c1 e6 04          	shl    $0x4,%rsi
    40003ef9:	48 8b 84 30 00 fd ff 	mov    -0x300(%rax,%rsi,1),%rax
    40003f00:	ff 
    40003f01:	89 04 24             	mov    %eax,(%rsp)
    40003f04:	f7 d0                	not    %eax
    40003f06:	c1 e8 1f             	shr    $0x1f,%eax
    40003f09:	89 c1                	mov    %eax,%ecx
    40003f0b:	0f be 43 04          	movsbl 0x4(%rbx),%eax
    40003f0f:	48 83 c3 04          	add    $0x4,%rbx
    40003f13:	e9 26 fd ff ff       	jmp    40003c3e <printf_core+0x2ae>
    40003f18:	44 89 1c 24          	mov    %r11d,(%rsp)
    40003f1c:	b9 01 00 00 00       	mov    $0x1,%ecx
    40003f21:	e9 8c fc ff ff       	jmp    40003bb2 <printf_core+0x222>
    40003f26:	44 89 f0             	mov    %r14d,%eax
    40003f29:	4d 85 c9             	test   %r9,%r9
    40003f2c:	0f 85 51 fd ff ff    	jne    40003c83 <printf_core+0x2f3>
    40003f32:	8b 74 24 0c          	mov    0xc(%rsp),%esi
    40003f36:	85 f6                	test   %esi,%esi
    40003f38:	0f 84 06 08 00 00    	je     40004744 <printf_core+0xdb4>
    40003f3e:	48 8b 7c 24 18       	mov    0x18(%rsp),%rdi
    40003f43:	4c 8b 4c 24 10       	mov    0x10(%rsp),%r9
    40003f48:	41 b8 01 00 00 00    	mov    $0x1,%r8d
    40003f4e:	4c 8b 54 24 20       	mov    0x20(%rsp),%r10
    40003f53:	48 83 c7 10          	add    $0x10,%rdi
    40003f57:	43 8b 34 82          	mov    (%r10,%r8,4),%esi
    40003f5b:	85 f6                	test   %esi,%esi
    40003f5d:	0f 84 14 08 00 00    	je     40004777 <printf_core+0xde7>
    40003f63:	4c 89 ca             	mov    %r9,%rdx
    40003f66:	49 83 c0 01          	add    $0x1,%r8
    40003f6a:	e8 71 e5 ff ff       	call   400024e0 <pop_arg>
    40003f6f:	48 83 c7 10          	add    $0x10,%rdi
    40003f73:	49 83 f8 0a          	cmp    $0xa,%r8
    40003f77:	75 de                	jne    40003f57 <printf_core+0x5c7>
    40003f79:	b8 01 00 00 00       	mov    $0x1,%eax
    40003f7e:	e9 00 fd ff ff       	jmp    40003c83 <printf_core+0x2f3>
    40003f83:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40003f88:	48 8d 84 24 88 00 00 	lea    0x88(%rsp),%rax
    40003f8f:	00 
    40003f90:	48 29 e8             	sub    %rbp,%rax
    40003f93:	48 89 44 24 28       	mov    %rax,0x28(%rsp)
    40003f98:	48 63 04 24          	movslq (%rsp),%rax
    40003f9c:	45 89 d8             	mov    %r11d,%r8d
    40003f9f:	48 39 44 24 28       	cmp    %rax,0x28(%rsp)
    40003fa4:	0f 8e 24 07 00 00    	jle    400046ce <printf_core+0xd3e>
    40003faa:	48 c7 44 24 38 6d 6c 	movq   $0x40006c6d,0x38(%rsp)
    40003fb1:	00 40 
    40003fb3:	ba ff ff ff 7f       	mov    $0x7fffffff,%edx
    40003fb8:	8b 44 24 28          	mov    0x28(%rsp),%eax
    40003fbc:	89 04 24             	mov    %eax,(%rsp)
    40003fbf:	39 14 24             	cmp    %edx,(%rsp)
    40003fc2:	0f 8f db fe ff ff    	jg     40003ea3 <printf_core+0x513>
    40003fc8:	8b 04 24             	mov    (%rsp),%eax
    40003fcb:	44 01 c0             	add    %r8d,%eax
    40003fce:	89 44 24 30          	mov    %eax,0x30(%rsp)
    40003fd2:	8b 4c 24 30          	mov    0x30(%rsp),%ecx
    40003fd6:	41 39 cf             	cmp    %ecx,%r15d
    40003fd9:	44 0f 4c f9          	cmovl  %ecx,%r15d
    40003fdd:	45 89 fa             	mov    %r15d,%r10d
    40003fe0:	45 39 fd             	cmp    %r15d,%r13d
    40003fe3:	0f 8c ba fe ff ff    	jl     40003ea3 <printf_core+0x513>
    40003fe9:	41 39 cf             	cmp    %ecx,%r15d
    40003fec:	41 0f 9e c7          	setle  %r15b
    40003ff0:	41 f7 c4 00 20 01 00 	test   $0x12000,%r12d
    40003ff7:	75 33                	jne    4000402c <printf_core+0x69c>
    40003ff9:	45 84 ff             	test   %r15b,%r15b
    40003ffc:	75 2e                	jne    4000402c <printf_core+0x69c>
    40003ffe:	44 89 d2             	mov    %r10d,%edx
    40004001:	4c 89 cf             	mov    %r9,%rdi
    40004004:	be 20 00 00 00       	mov    $0x20,%esi
    40004009:	44 89 44 24 4c       	mov    %r8d,0x4c(%rsp)
    4000400e:	44 89 54 24 48       	mov    %r10d,0x48(%rsp)
    40004013:	4c 89 4c 24 40       	mov    %r9,0x40(%rsp)
    40004018:	e8 a3 e6 ff ff       	call   400026c0 <pad.part.0>
    4000401d:	44 8b 44 24 4c       	mov    0x4c(%rsp),%r8d
    40004022:	44 8b 54 24 48       	mov    0x48(%rsp),%r10d
    40004027:	4c 8b 4c 24 40       	mov    0x40(%rsp),%r9
    4000402c:	41 f6 01 20          	testb  $0x20,(%r9)
    40004030:	0f 84 a1 05 00 00    	je     400045d7 <printf_core+0xc47>
    40004036:	44 89 e0             	mov    %r12d,%eax
    40004039:	35 00 00 01 00       	xor    $0x10000,%eax
    4000403e:	a9 00 20 01 00       	test   $0x12000,%eax
    40004043:	75 2d                	jne    40004072 <printf_core+0x6e2>
    40004045:	45 84 ff             	test   %r15b,%r15b
    40004048:	75 28                	jne    40004072 <printf_core+0x6e2>
    4000404a:	8b 4c 24 30          	mov    0x30(%rsp),%ecx
    4000404e:	44 89 d2             	mov    %r10d,%edx
    40004051:	4c 89 cf             	mov    %r9,%rdi
    40004054:	be 30 00 00 00       	mov    $0x30,%esi
    40004059:	44 89 54 24 40       	mov    %r10d,0x40(%rsp)
    4000405e:	4c 89 4c 24 38       	mov    %r9,0x38(%rsp)
    40004063:	e8 58 e6 ff ff       	call   400026c0 <pad.part.0>
    40004068:	44 8b 54 24 40       	mov    0x40(%rsp),%r10d
    4000406d:	4c 8b 4c 24 38       	mov    0x38(%rsp),%r9
    40004072:	48 8b 44 24 28       	mov    0x28(%rsp),%rax
    40004077:	8b 14 24             	mov    (%rsp),%edx
    4000407a:	89 c1                	mov    %eax,%ecx
    4000407c:	39 d0                	cmp    %edx,%eax
    4000407e:	7d 1f                	jge    4000409f <printf_core+0x70f>
    40004080:	4c 89 cf             	mov    %r9,%rdi
    40004083:	be 30 00 00 00       	mov    $0x30,%esi
    40004088:	44 89 54 24 38       	mov    %r10d,0x38(%rsp)
    4000408d:	4c 89 0c 24          	mov    %r9,(%rsp)
    40004091:	e8 2a e6 ff ff       	call   400026c0 <pad.part.0>
    40004096:	44 8b 54 24 38       	mov    0x38(%rsp),%r10d
    4000409b:	4c 8b 0c 24          	mov    (%rsp),%r9
    4000409f:	41 f6 01 20          	testb  $0x20,(%r9)
    400040a3:	0f 84 07 05 00 00    	je     400045b0 <printf_core+0xc20>
    400040a9:	41 81 f4 00 20 00 00 	xor    $0x2000,%r12d
    400040b0:	41 81 e4 00 20 01 00 	and    $0x12000,%r12d
    400040b7:	0f 85 8e f9 ff ff    	jne    40003a4b <printf_core+0xbb>
    400040bd:	45 84 ff             	test   %r15b,%r15b
    400040c0:	0f 85 85 f9 ff ff    	jne    40003a4b <printf_core+0xbb>
    400040c6:	8b 4c 24 30          	mov    0x30(%rsp),%ecx
    400040ca:	44 89 d2             	mov    %r10d,%edx
    400040cd:	4c 89 cf             	mov    %r9,%rdi
    400040d0:	be 20 00 00 00       	mov    $0x20,%esi
    400040d5:	44 89 54 24 28       	mov    %r10d,0x28(%rsp)
    400040da:	4c 89 0c 24          	mov    %r9,(%rsp)
    400040de:	e8 dd e5 ff ff       	call   400026c0 <pad.part.0>
    400040e3:	4c 8b 0c 24          	mov    (%rsp),%r9
    400040e7:	44 8b 54 24 28       	mov    0x28(%rsp),%r10d
    400040ec:	e9 5a f9 ff ff       	jmp    40003a4b <printf_core+0xbb>
    400040f1:	44 8b 14 24          	mov    (%rsp),%r10d
    400040f5:	45 85 d2             	test   %r10d,%r10d
    400040f8:	79 09                	jns    40004103 <printf_core+0x773>
    400040fa:	83 e1 01             	and    $0x1,%ecx
    400040fd:	0f 85 a0 fd ff ff    	jne    40003ea3 <printf_core+0x513>
    40004103:	ff 74 24 68          	push   0x68(%rsp)
    40004107:	41 89 d0             	mov    %edx,%r8d
    4000410a:	4c 89 cf             	mov    %r9,%rdi
    4000410d:	44 89 e1             	mov    %r12d,%ecx
    40004110:	ff 74 24 68          	push   0x68(%rsp)
    40004114:	44 89 fe             	mov    %r15d,%esi
    40004117:	8b 54 24 10          	mov    0x10(%rsp),%edx
    4000411b:	4c 89 4c 24 10       	mov    %r9,0x10(%rsp)
    40004120:	e8 5b e6 ff ff       	call   40002780 <fmt_fp>
    40004125:	5f                   	pop    %rdi
    40004126:	41 58                	pop    %r8
    40004128:	85 c0                	test   %eax,%eax
    4000412a:	4c 8b 0c 24          	mov    (%rsp),%r9
    4000412e:	41 89 c2             	mov    %eax,%r10d
    40004131:	0f 89 0b f9 ff ff    	jns    40003a42 <printf_core+0xb2>
    40004137:	e9 67 fd ff ff       	jmp    40003ea3 <printf_core+0x513>
    4000413c:	0f 1f 40 00          	nopl   0x0(%rax)
    40004140:	89 d7                	mov    %edx,%edi
    40004142:	83 e7 20             	and    $0x20,%edi
    40004145:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
    4000414a:	48 85 c0             	test   %rax,%rax
    4000414d:	0f 84 f8 05 00 00    	je     4000474b <printf_core+0xdbb>
    40004153:	4c 8d 94 24 88 00 00 	lea    0x88(%rsp),%r10
    4000415a:	00 
    4000415b:	4c 89 d5             	mov    %r10,%rbp
    4000415e:	66 90                	xchg   %ax,%ax
    40004160:	48 89 c6             	mov    %rax,%rsi
    40004163:	48 83 ed 01          	sub    $0x1,%rbp
    40004167:	83 e6 0f             	and    $0xf,%esi
    4000416a:	44 0f b6 86 80 6e 00 	movzbl 0x40006e80(%rsi),%r8d
    40004171:	40 
    40004172:	41 09 f8             	or     %edi,%r8d
    40004175:	48 c1 e8 04          	shr    $0x4,%rax
    40004179:	44 88 45 00          	mov    %r8b,0x0(%rbp)
    4000417d:	75 e1                	jne    40004160 <printf_core+0x7d0>
    4000417f:	41 f6 c4 08          	test   $0x8,%r12b
    40004183:	0f 84 d5 04 00 00    	je     4000465e <printf_core+0xcce>
    40004189:	89 d0                	mov    %edx,%eax
    4000418b:	41 b8 02 00 00 00    	mov    $0x2,%r8d
    40004191:	c1 f8 04             	sar    $0x4,%eax
    40004194:	48 98                	cltq   
    40004196:	48 05 6d 6c 00 40    	add    $0x40006c6d,%rax
    4000419c:	48 89 44 24 38       	mov    %rax,0x38(%rsp)
    400041a1:	8b 34 24             	mov    (%rsp),%esi
    400041a4:	85 f6                	test   %esi,%esi
    400041a6:	79 09                	jns    400041b1 <printf_core+0x821>
    400041a8:	f6 c1 01             	test   $0x1,%cl
    400041ab:	0f 85 f2 fc ff ff    	jne    40003ea3 <printf_core+0x513>
    400041b1:	44 89 e0             	mov    %r12d,%eax
    400041b4:	8b 14 24             	mov    (%rsp),%edx
    400041b7:	25 ff ff fe ff       	and    $0xfffeffff,%eax
    400041bc:	85 c9                	test   %ecx,%ecx
    400041be:	44 0f 45 e0          	cmovne %eax,%r12d
    400041c2:	48 83 7c 24 60 00    	cmpq   $0x0,0x60(%rsp)
    400041c8:	0f 94 c0             	sete   %al
    400041cb:	85 d2                	test   %edx,%edx
    400041cd:	75 08                	jne    400041d7 <printf_core+0x847>
    400041cf:	84 c0                	test   %al,%al
    400041d1:	0f 85 98 04 00 00    	jne    4000466f <printf_core+0xcdf>
    400041d7:	4c 89 d7             	mov    %r10,%rdi
    400041da:	48 63 14 24          	movslq (%rsp),%rdx
    400041de:	0f b6 c0             	movzbl %al,%eax
    400041e1:	48 29 ef             	sub    %rbp,%rdi
    400041e4:	48 01 f8             	add    %rdi,%rax
    400041e7:	48 89 7c 24 28       	mov    %rdi,0x28(%rsp)
    400041ec:	48 39 d0             	cmp    %rdx,%rax
    400041ef:	48 0f 4c c2          	cmovl  %rdx,%rax
    400041f3:	ba ff ff ff 7f       	mov    $0x7fffffff,%edx
    400041f8:	44 29 c2             	sub    %r8d,%edx
    400041fb:	89 04 24             	mov    %eax,(%rsp)
    400041fe:	48 98                	cltq   
    40004200:	48 39 c7             	cmp    %rax,%rdi
    40004203:	0f 8f af fd ff ff    	jg     40003fb8 <printf_core+0x628>
    40004209:	e9 b1 fd ff ff       	jmp    40003fbf <printf_core+0x62f>
    4000420e:	48 8b 74 24 60       	mov    0x60(%rsp),%rsi
    40004213:	48 85 f6             	test   %rsi,%rsi
    40004216:	0f 88 ce 04 00 00    	js     400046ea <printf_core+0xd5a>
    4000421c:	41 f7 c4 00 08 00 00 	test   $0x800,%r12d
    40004223:	0f 85 07 05 00 00    	jne    40004730 <printf_core+0xda0>
    40004229:	41 f6 c4 01          	test   $0x1,%r12b
    4000422d:	ba 6f 6c 00 40       	mov    $0x40006c6f,%edx
    40004232:	b8 6d 6c 00 40       	mov    $0x40006c6d,%eax
    40004237:	41 b8 01 00 00 00    	mov    $0x1,%r8d
    4000423d:	48 0f 45 c2          	cmovne %rdx,%rax
    40004241:	45 0f 44 c3          	cmove  %r11d,%r8d
    40004245:	48 89 44 24 38       	mov    %rax,0x38(%rsp)
    4000424a:	48 85 f6             	test   %rsi,%rsi
    4000424d:	0f 84 14 05 00 00    	je     40004767 <printf_core+0xdd7>
    40004253:	4c 8d 94 24 88 00 00 	lea    0x88(%rsp),%r10
    4000425a:	00 
    4000425b:	4c 89 d5             	mov    %r10,%rbp
    4000425e:	66 90                	xchg   %ax,%ax
    40004260:	48 b8 cd cc cc cc cc 	movabs $0xcccccccccccccccd,%rax
    40004267:	cc cc cc 
    4000426a:	48 83 ed 01          	sub    $0x1,%rbp
    4000426e:	48 f7 e6             	mul    %rsi
    40004271:	48 89 f0             	mov    %rsi,%rax
    40004274:	48 c1 ea 03          	shr    $0x3,%rdx
    40004278:	48 8d 3c 92          	lea    (%rdx,%rdx,4),%rdi
    4000427c:	48 01 ff             	add    %rdi,%rdi
    4000427f:	48 29 f8             	sub    %rdi,%rax
    40004282:	83 c0 30             	add    $0x30,%eax
    40004285:	88 45 00             	mov    %al,0x0(%rbp)
    40004288:	48 89 f0             	mov    %rsi,%rax
    4000428b:	48 89 d6             	mov    %rdx,%rsi
    4000428e:	48 83 f8 09          	cmp    $0x9,%rax
    40004292:	77 cc                	ja     40004260 <printf_core+0x8d0>
    40004294:	e9 08 ff ff ff       	jmp    400041a1 <printf_core+0x811>
    40004299:	4c 89 4c 24 30       	mov    %r9,0x30(%rsp)
    4000429e:	4c 89 5c 24 28       	mov    %r11,0x28(%rsp)
    400042a3:	e8 98 0d 00 00       	call   40005040 <__errno_location>
    400042a8:	8b 38                	mov    (%rax),%edi
    400042aa:	e8 d1 0d 00 00       	call   40005080 <strerror>
    400042af:	4c 8b 5c 24 28       	mov    0x28(%rsp),%r11
    400042b4:	4c 8b 4c 24 30       	mov    0x30(%rsp),%r9
    400042b9:	48 89 c5             	mov    %rax,%rbp
    400042bc:	8b 04 24             	mov    (%rsp),%eax
    400042bf:	85 c0                	test   %eax,%eax
    400042c1:	0f 88 af fb ff ff    	js     40003e76 <printf_core+0x4e6>
    400042c7:	48 63 34 24          	movslq (%rsp),%rsi
    400042cb:	48 89 ef             	mov    %rbp,%rdi
    400042ce:	4c 89 4c 24 30       	mov    %r9,0x30(%rsp)
    400042d3:	4c 89 5c 24 28       	mov    %r11,0x28(%rsp)
    400042d8:	e8 a3 06 00 00       	call   40004980 <strnlen>
    400042dd:	4c 8b 5c 24 28       	mov    0x28(%rsp),%r11
    400042e2:	4c 8b 4c 24 30       	mov    0x30(%rsp),%r9
    400042e7:	48 8d 54 05 00       	lea    0x0(%rbp,%rax,1),%rdx
    400042ec:	48 29 ea             	sub    %rbp,%rdx
    400042ef:	89 04 24             	mov    %eax,(%rsp)
    400042f2:	41 81 e4 ff ff fe ff 	and    $0xfffeffff,%r12d
    400042f9:	48 89 54 24 28       	mov    %rdx,0x28(%rsp)
    400042fe:	e9 95 fc ff ff       	jmp    40003f98 <printf_core+0x608>
    40004303:	41 83 f8 07          	cmp    $0x7,%r8d
    40004307:	0f 87 35 f7 ff ff    	ja     40003a42 <printf_core+0xb2>
    4000430d:	ff 24 c5 40 6e 00 40 	jmp    *0x40006e40(,%rax,8)
    40004314:	48 8b 54 24 60       	mov    0x60(%rsp),%rdx
    40004319:	49 63 c6             	movslq %r14d,%rax
    4000431c:	48 89 02             	mov    %rax,(%rdx)
    4000431f:	e9 27 f7 ff ff       	jmp    40003a4b <printf_core+0xbb>
    40004324:	8b 14 24             	mov    (%rsp),%edx
    40004327:	b8 10 00 00 00       	mov    $0x10,%eax
    4000432c:	bf 20 00 00 00       	mov    $0x20,%edi
    40004331:	39 c2                	cmp    %eax,%edx
    40004333:	0f 43 c2             	cmovae %edx,%eax
    40004336:	41 83 cc 08          	or     $0x8,%r12d
    4000433a:	ba 78 00 00 00       	mov    $0x78,%edx
    4000433f:	89 04 24             	mov    %eax,(%rsp)
    40004342:	e9 fe fd ff ff       	jmp    40004145 <printf_core+0x7b5>
    40004347:	48 8b 6c 24 60       	mov    0x60(%rsp),%rbp
    4000434c:	b8 77 6c 00 40       	mov    $0x40006c77,%eax
    40004351:	48 85 ed             	test   %rbp,%rbp
    40004354:	48 0f 44 e8          	cmove  %rax,%rbp
    40004358:	e9 5f ff ff ff       	jmp    400042bc <printf_core+0x92c>
    4000435d:	48 8b 74 24 60       	mov    0x60(%rsp),%rsi
    40004362:	48 85 f6             	test   %rsi,%rsi
    40004365:	0f 85 9b 03 00 00    	jne    40004706 <printf_core+0xd76>
    4000436b:	40 88 b4 24 87 00 00 	mov    %sil,0x87(%rsp)
    40004372:	00 
    40004373:	41 81 e4 ff ff fe ff 	and    $0xfffeffff,%r12d
    4000437a:	45 89 d8             	mov    %r11d,%r8d
    4000437d:	48 8d ac 24 87 00 00 	lea    0x87(%rsp),%rbp
    40004384:	00 
    40004385:	c7 44 24 30 01 00 00 	movl   $0x1,0x30(%rsp)
    4000438c:	00 
    4000438d:	48 c7 44 24 28 01 00 	movq   $0x1,0x28(%rsp)
    40004394:	00 00 
    40004396:	48 c7 44 24 38 6d 6c 	movq   $0x40006c6d,0x38(%rsp)
    4000439d:	00 40 
    4000439f:	c7 04 24 01 00 00 00 	movl   $0x1,(%rsp)
    400043a6:	e9 27 fc ff ff       	jmp    40003fd2 <printf_core+0x642>
    400043ab:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
    400043b0:	48 89 44 24 28       	mov    %rax,0x28(%rsp)
    400043b5:	48 63 04 24          	movslq (%rsp),%rax
    400043b9:	48 89 c7             	mov    %rax,%rdi
    400043bc:	48 85 c0             	test   %rax,%rax
    400043bf:	0f 84 d8 03 00 00    	je     4000479d <printf_core+0xe0d>
    400043c5:	48 8b 44 24 28       	mov    0x28(%rsp),%rax
    400043ca:	44 8b 10             	mov    (%rax),%r10d
    400043cd:	48 8b 54 24 28       	mov    0x28(%rsp),%rdx
    400043d2:	48 89 1c 24          	mov    %rbx,(%rsp)
    400043d6:	31 ed                	xor    %ebp,%ebp
    400043d8:	48 89 fb             	mov    %rdi,%rbx
    400043db:	44 89 64 24 30       	mov    %r12d,0x30(%rsp)
    400043e0:	44 89 d6             	mov    %r10d,%esi
    400043e3:	4c 89 4c 24 38       	mov    %r9,0x38(%rsp)
    400043e8:	49 89 d4             	mov    %rdx,%r12
    400043eb:	eb 32                	jmp    4000441f <printf_core+0xa8f>
    400043ed:	0f 1f 00             	nopl   (%rax)
    400043f0:	48 8d 7c 24 54       	lea    0x54(%rsp),%rdi
    400043f5:	49 83 c4 04          	add    $0x4,%r12
    400043f9:	e8 e2 10 00 00       	call   400054e0 <wctomb>
    400043fe:	85 c0                	test   %eax,%eax
    40004400:	0f 88 a8 fa ff ff    	js     40003eae <printf_core+0x51e>
    40004406:	48 89 de             	mov    %rbx,%rsi
    40004409:	48 98                	cltq   
    4000440b:	48 29 ee             	sub    %rbp,%rsi
    4000440e:	48 39 f0             	cmp    %rsi,%rax
    40004411:	77 10                	ja     40004423 <printf_core+0xa93>
    40004413:	48 01 c5             	add    %rax,%rbp
    40004416:	48 39 eb             	cmp    %rbp,%rbx
    40004419:	76 08                	jbe    40004423 <printf_core+0xa93>
    4000441b:	41 8b 34 24          	mov    (%r12),%esi
    4000441f:	85 f6                	test   %esi,%esi
    40004421:	75 cd                	jne    400043f0 <printf_core+0xa60>
    40004423:	b8 00 00 00 80       	mov    $0x80000000,%eax
    40004428:	48 8b 1c 24          	mov    (%rsp),%rbx
    4000442c:	44 8b 64 24 30       	mov    0x30(%rsp),%r12d
    40004431:	4c 8b 4c 24 38       	mov    0x38(%rsp),%r9
    40004436:	48 39 c5             	cmp    %rax,%rbp
    40004439:	0f 83 64 fa ff ff    	jae    40003ea3 <printf_core+0x513>
    4000443f:	41 39 ef             	cmp    %ebp,%r15d
    40004442:	41 89 ea             	mov    %ebp,%r10d
    40004445:	0f 9e 04 24          	setle  (%rsp)
    40004449:	0f b6 04 24          	movzbl (%rsp),%eax
    4000444d:	41 f7 c4 00 20 01 00 	test   $0x12000,%r12d
    40004454:	75 2f                	jne    40004485 <printf_core+0xaf5>
    40004456:	84 c0                	test   %al,%al
    40004458:	75 2b                	jne    40004485 <printf_core+0xaf5>
    4000445a:	44 89 d1             	mov    %r10d,%ecx
    4000445d:	4c 89 cf             	mov    %r9,%rdi
    40004460:	44 89 fa             	mov    %r15d,%edx
    40004463:	be 20 00 00 00       	mov    $0x20,%esi
    40004468:	44 89 54 24 38       	mov    %r10d,0x38(%rsp)
    4000446d:	4c 89 4c 24 30       	mov    %r9,0x30(%rsp)
    40004472:	e8 49 e2 ff ff       	call   400026c0 <pad.part.0>
    40004477:	c6 04 24 00          	movb   $0x0,(%rsp)
    4000447b:	44 8b 54 24 38       	mov    0x38(%rsp),%r10d
    40004480:	4c 8b 4c 24 30       	mov    0x30(%rsp),%r9
    40004485:	31 c9                	xor    %ecx,%ecx
    40004487:	48 85 ed             	test   %rbp,%rbp
    4000448a:	0f 84 87 01 00 00    	je     40004617 <printf_core+0xc87>
    40004490:	44 89 6c 24 30       	mov    %r13d,0x30(%rsp)
    40004495:	4d 89 cd             	mov    %r9,%r13
    40004498:	48 89 5c 24 38       	mov    %rbx,0x38(%rsp)
    4000449d:	48 89 cb             	mov    %rcx,%rbx
    400044a0:	44 89 64 24 40       	mov    %r12d,0x40(%rsp)
    400044a5:	4c 8b 64 24 28       	mov    0x28(%rsp),%r12
    400044aa:	44 89 54 24 48       	mov    %r10d,0x48(%rsp)
    400044af:	eb 10                	jmp    400044c1 <printf_core+0xb31>
    400044b1:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    400044b8:	48 39 dd             	cmp    %rbx,%rbp
    400044bb:	0f 86 3f 01 00 00    	jbe    40004600 <printf_core+0xc70>
    400044c1:	41 8b 34 24          	mov    (%r12),%esi
    400044c5:	85 f6                	test   %esi,%esi
    400044c7:	0f 84 33 01 00 00    	je     40004600 <printf_core+0xc70>
    400044cd:	48 8d 7c 24 54       	lea    0x54(%rsp),%rdi
    400044d2:	49 83 c4 04          	add    $0x4,%r12
    400044d6:	e8 05 10 00 00       	call   400054e0 <wctomb>
    400044db:	48 63 f0             	movslq %eax,%rsi
    400044de:	48 01 f3             	add    %rsi,%rbx
    400044e1:	48 39 dd             	cmp    %rbx,%rbp
    400044e4:	0f 82 16 01 00 00    	jb     40004600 <printf_core+0xc70>
    400044ea:	41 f6 45 00 20       	testb  $0x20,0x0(%r13)
    400044ef:	75 c7                	jne    400044b8 <printf_core+0xb28>
    400044f1:	4c 89 ea             	mov    %r13,%rdx
    400044f4:	48 8d 7c 24 54       	lea    0x54(%rsp),%rdi
    400044f9:	e8 32 14 00 00       	call   40005930 <__fwritex>
    400044fe:	eb b8                	jmp    400044b8 <printf_core+0xb28>
    40004500:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
    40004505:	4c 8d 94 24 88 00 00 	lea    0x88(%rsp),%r10
    4000450c:	00 
    4000450d:	4c 89 d5             	mov    %r10,%rbp
    40004510:	48 85 c0             	test   %rax,%rax
    40004513:	74 18                	je     4000452d <printf_core+0xb9d>
    40004515:	0f 1f 00             	nopl   (%rax)
    40004518:	89 c2                	mov    %eax,%edx
    4000451a:	48 83 ed 01          	sub    $0x1,%rbp
    4000451e:	83 e2 07             	and    $0x7,%edx
    40004521:	83 c2 30             	add    $0x30,%edx
    40004524:	48 c1 e8 03          	shr    $0x3,%rax
    40004528:	88 55 00             	mov    %dl,0x0(%rbp)
    4000452b:	75 eb                	jne    40004518 <printf_core+0xb88>
    4000452d:	41 f6 c4 08          	test   $0x8,%r12b
    40004531:	0f 84 27 01 00 00    	je     4000465e <printf_core+0xcce>
    40004537:	4c 89 d0             	mov    %r10,%rax
    4000453a:	48 63 14 24          	movslq (%rsp),%rdx
    4000453e:	48 29 e8             	sub    %rbp,%rax
    40004541:	48 39 d0             	cmp    %rdx,%rax
    40004544:	0f 8c 14 01 00 00    	jl     4000465e <printf_core+0xcce>
    4000454a:	48 c7 44 24 38 6d 6c 	movq   $0x40006c6d,0x38(%rsp)
    40004551:	00 40 
    40004553:	83 c0 01             	add    $0x1,%eax
    40004556:	45 89 d8             	mov    %r11d,%r8d
    40004559:	89 04 24             	mov    %eax,(%rsp)
    4000455c:	e9 40 fc ff ff       	jmp    400041a1 <printf_core+0x811>
    40004561:	48 c7 44 24 38 6d 6c 	movq   $0x40006c6d,0x38(%rsp)
    40004568:	00 40 
    4000456a:	48 8b 74 24 60       	mov    0x60(%rsp),%rsi
    4000456f:	45 89 d8             	mov    %r11d,%r8d
    40004572:	e9 d3 fc ff ff       	jmp    4000424a <printf_core+0x8ba>
    40004577:	48 8b 74 24 60       	mov    0x60(%rsp),%rsi
    4000457c:	e9 ea fd ff ff       	jmp    4000436b <printf_core+0x9db>
    40004581:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
    40004586:	44 89 30             	mov    %r14d,(%rax)
    40004589:	e9 bd f4 ff ff       	jmp    40003a4b <printf_core+0xbb>
    4000458e:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
    40004593:	66 44 89 30          	mov    %r14w,(%rax)
    40004597:	e9 af f4 ff ff       	jmp    40003a4b <printf_core+0xbb>
    4000459c:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
    400045a1:	44 88 30             	mov    %r14b,(%rax)
    400045a4:	e9 a2 f4 ff ff       	jmp    40003a4b <printf_core+0xbb>
    400045a9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    400045b0:	48 8b 74 24 28       	mov    0x28(%rsp),%rsi
    400045b5:	4c 89 ca             	mov    %r9,%rdx
    400045b8:	48 89 ef             	mov    %rbp,%rdi
    400045bb:	44 89 54 24 38       	mov    %r10d,0x38(%rsp)
    400045c0:	4c 89 0c 24          	mov    %r9,(%rsp)
    400045c4:	e8 67 13 00 00       	call   40005930 <__fwritex>
    400045c9:	44 8b 54 24 38       	mov    0x38(%rsp),%r10d
    400045ce:	4c 8b 0c 24          	mov    (%rsp),%r9
    400045d2:	e9 d2 fa ff ff       	jmp    400040a9 <printf_core+0x719>
    400045d7:	48 8b 7c 24 38       	mov    0x38(%rsp),%rdi
    400045dc:	4c 89 ca             	mov    %r9,%rdx
    400045df:	49 63 f0             	movslq %r8d,%rsi
    400045e2:	44 89 54 24 48       	mov    %r10d,0x48(%rsp)
    400045e7:	4c 89 4c 24 40       	mov    %r9,0x40(%rsp)
    400045ec:	e8 3f 13 00 00       	call   40005930 <__fwritex>
    400045f1:	44 8b 54 24 48       	mov    0x48(%rsp),%r10d
    400045f6:	4c 8b 4c 24 40       	mov    0x40(%rsp),%r9
    400045fb:	e9 36 fa ff ff       	jmp    40004036 <printf_core+0x6a6>
    40004600:	4d 89 e9             	mov    %r13,%r9
    40004603:	48 8b 5c 24 38       	mov    0x38(%rsp),%rbx
    40004608:	44 8b 64 24 40       	mov    0x40(%rsp),%r12d
    4000460d:	44 8b 54 24 48       	mov    0x48(%rsp),%r10d
    40004612:	44 8b 6c 24 30       	mov    0x30(%rsp),%r13d
    40004617:	41 81 f4 00 20 00 00 	xor    $0x2000,%r12d
    4000461e:	41 81 e4 00 20 01 00 	and    $0x12000,%r12d
    40004625:	75 2b                	jne    40004652 <printf_core+0xcc2>
    40004627:	80 3c 24 00          	cmpb   $0x0,(%rsp)
    4000462b:	75 25                	jne    40004652 <printf_core+0xcc2>
    4000462d:	44 89 d1             	mov    %r10d,%ecx
    40004630:	4c 89 cf             	mov    %r9,%rdi
    40004633:	44 89 fa             	mov    %r15d,%edx
    40004636:	be 20 00 00 00       	mov    $0x20,%esi
    4000463b:	44 89 54 24 28       	mov    %r10d,0x28(%rsp)
    40004640:	4c 89 0c 24          	mov    %r9,(%rsp)
    40004644:	e8 77 e0 ff ff       	call   400026c0 <pad.part.0>
    40004649:	44 8b 54 24 28       	mov    0x28(%rsp),%r10d
    4000464e:	4c 8b 0c 24          	mov    (%rsp),%r9
    40004652:	45 39 fa             	cmp    %r15d,%r10d
    40004655:	45 0f 4c d7          	cmovl  %r15d,%r10d
    40004659:	e9 e4 f3 ff ff       	jmp    40003a42 <printf_core+0xb2>
    4000465e:	48 c7 44 24 38 6d 6c 	movq   $0x40006c6d,0x38(%rsp)
    40004665:	00 40 
    40004667:	45 89 d8             	mov    %r11d,%r8d
    4000466a:	e9 32 fb ff ff       	jmp    400041a1 <printf_core+0x811>
    4000466f:	44 89 44 24 30       	mov    %r8d,0x30(%rsp)
    40004674:	4c 89 d5             	mov    %r10,%rbp
    40004677:	44 89 1c 24          	mov    %r11d,(%rsp)
    4000467b:	48 c7 44 24 28 00 00 	movq   $0x0,0x28(%rsp)
    40004682:	00 00 
    40004684:	e9 49 f9 ff ff       	jmp    40003fd2 <printf_core+0x642>
    40004689:	48 8b 7c 24 20       	mov    0x20(%rsp),%rdi
    4000468e:	c7 44 24 0c 01 00 00 	movl   $0x1,0xc(%rsp)
    40004695:	00 
    40004696:	45 89 df             	mov    %r11d,%r15d
    40004699:	c7 84 b7 40 ff ff ff 	movl   $0xa,-0xc0(%rdi,%rsi,4)
    400046a0:	0a 00 00 00 
    400046a4:	0f b6 73 03          	movzbl 0x3(%rbx),%esi
    400046a8:	48 89 c3             	mov    %rax,%rbx
    400046ab:	e9 90 f4 ff ff       	jmp    40003b40 <printf_core+0x1b0>
    400046b0:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
    400046b5:	44 89 1c 24          	mov    %r11d,(%rsp)
    400046b9:	b9 01 00 00 00       	mov    $0x1,%ecx
    400046be:	c7 84 b0 40 ff ff ff 	movl   $0xa,-0xc0(%rax,%rsi,4)
    400046c5:	0a 00 00 00 
    400046c9:	e9 3d f8 ff ff       	jmp    40003f0b <printf_core+0x57b>
    400046ce:	48 c7 44 24 38 6d 6c 	movq   $0x40006c6d,0x38(%rsp)
    400046d5:	00 40 
    400046d7:	e9 ec f8 ff ff       	jmp    40003fc8 <printf_core+0x638>
    400046dc:	44 89 1c 24          	mov    %r11d,(%rsp)
    400046e0:	b9 01 00 00 00       	mov    $0x1,%ecx
    400046e5:	e9 54 f5 ff ff       	jmp    40003c3e <printf_core+0x2ae>
    400046ea:	48 f7 de             	neg    %rsi
    400046ed:	41 b8 01 00 00 00    	mov    $0x1,%r8d
    400046f3:	48 c7 44 24 38 6d 6c 	movq   $0x40006c6d,0x38(%rsp)
    400046fa:	00 40 
    400046fc:	48 89 74 24 60       	mov    %rsi,0x60(%rsp)
    40004701:	e9 4d fb ff ff       	jmp    40004253 <printf_core+0x8c3>
    40004706:	48 8d 44 24 58       	lea    0x58(%rsp),%rax
    4000470b:	89 74 24 58          	mov    %esi,0x58(%rsp)
    4000470f:	41 89 f2             	mov    %esi,%r10d
    40004712:	48 c7 c7 ff ff ff ff 	mov    $0xffffffffffffffff,%rdi
    40004719:	c7 44 24 5c 00 00 00 	movl   $0x0,0x5c(%rsp)
    40004720:	00 
    40004721:	48 89 44 24 60       	mov    %rax,0x60(%rsp)
    40004726:	48 89 44 24 28       	mov    %rax,0x28(%rsp)
    4000472b:	e9 9d fc ff ff       	jmp    400043cd <printf_core+0xa3d>
    40004730:	48 c7 44 24 38 6e 6c 	movq   $0x40006c6e,0x38(%rsp)
    40004737:	00 40 
    40004739:	41 b8 01 00 00 00    	mov    $0x1,%r8d
    4000473f:	e9 06 fb ff ff       	jmp    4000424a <printf_core+0x8ba>
    40004744:	31 c0                	xor    %eax,%eax
    40004746:	e9 38 f5 ff ff       	jmp    40003c83 <printf_core+0x2f3>
    4000474b:	4c 8d 94 24 88 00 00 	lea    0x88(%rsp),%r10
    40004752:	00 
    40004753:	45 89 d8             	mov    %r11d,%r8d
    40004756:	48 c7 44 24 38 6d 6c 	movq   $0x40006c6d,0x38(%rsp)
    4000475d:	00 40 
    4000475f:	4c 89 d5             	mov    %r10,%rbp
    40004762:	e9 3a fa ff ff       	jmp    400041a1 <printf_core+0x811>
    40004767:	4c 8d 94 24 88 00 00 	lea    0x88(%rsp),%r10
    4000476e:	00 
    4000476f:	4c 89 d5             	mov    %r10,%rbp
    40004772:	e9 2a fa ff ff       	jmp    400041a1 <printf_core+0x811>
    40004777:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
    4000477c:	4a 8d 14 80          	lea    (%rax,%r8,4),%rdx
    40004780:	48 83 c0 24          	add    $0x24,%rax
    40004784:	48 39 d0             	cmp    %rdx,%rax
    40004787:	0f 84 ec f7 ff ff    	je     40003f79 <printf_core+0x5e9>
    4000478d:	8b 4a 04             	mov    0x4(%rdx),%ecx
    40004790:	48 83 c2 04          	add    $0x4,%rdx
    40004794:	85 c9                	test   %ecx,%ecx
    40004796:	74 ec                	je     40004784 <printf_core+0xdf4>
    40004798:	e9 d6 f4 ff ff       	jmp    40003c73 <printf_core+0x2e3>
    4000479d:	45 85 ff             	test   %r15d,%r15d
    400047a0:	0f 9e c0             	setle  %al
    400047a3:	88 04 24             	mov    %al,(%rsp)
    400047a6:	41 f7 c4 00 20 01 00 	test   $0x12000,%r12d
    400047ad:	0f 85 64 fe ff ff    	jne    40004617 <printf_core+0xc87>
    400047b3:	31 ed                	xor    %ebp,%ebp
    400047b5:	84 c0                	test   %al,%al
    400047b7:	0f 84 9d fc ff ff    	je     4000445a <printf_core+0xaca>
    400047bd:	e9 55 fe ff ff       	jmp    40004617 <printf_core+0xc87>
    400047c2:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    400047c9:	00 00 00 
    400047cc:	0f 1f 40 00          	nopl   0x0(%rax)

00000000400047d0 <vfprintf>:
    400047d0:	41 57                	push   %r15
    400047d2:	66 0f ef c0          	pxor   %xmm0,%xmm0
    400047d6:	41 56                	push   %r14
    400047d8:	49 89 f6             	mov    %rsi,%r14
    400047db:	41 55                	push   %r13
    400047dd:	41 54                	push   %r12
    400047df:	55                   	push   %rbp
    400047e0:	48 89 fd             	mov    %rdi,%rbp
    400047e3:	31 ff                	xor    %edi,%edi
    400047e5:	53                   	push   %rbx
    400047e6:	48 81 ec 48 01 00 00 	sub    $0x148,%rsp
    400047ed:	48 c7 44 24 40 00 00 	movq   $0x0,0x40(%rsp)
    400047f4:	00 00 
    400047f6:	4c 8d 44 24 20       	lea    0x20(%rsp),%r8
    400047fb:	48 8d 8c 24 a0 00 00 	lea    0xa0(%rsp),%rcx
    40004802:	00 
    40004803:	0f 29 44 24 20       	movaps %xmm0,0x20(%rsp)
    40004808:	0f 29 44 24 30       	movaps %xmm0,0x30(%rsp)
    4000480d:	f3 0f 6f 0a          	movdqu (%rdx),%xmm1
    40004811:	0f 11 4c 24 08       	movups %xmm1,0x8(%rsp)
    40004816:	48 8b 42 10          	mov    0x10(%rdx),%rax
    4000481a:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
    4000481f:	48 89 44 24 18       	mov    %rax,0x18(%rsp)
    40004824:	e8 67 f1 ff ff       	call   40003990 <printf_core>
    40004829:	85 c0                	test   %eax,%eax
    4000482b:	0f 88 29 01 00 00    	js     4000495a <vfprintf+0x18a>
    40004831:	8b 85 8c 00 00 00    	mov    0x8c(%rbp),%eax
    40004837:	45 31 ed             	xor    %r13d,%r13d
    4000483a:	85 c0                	test   %eax,%eax
    4000483c:	78 0b                	js     40004849 <vfprintf+0x79>
    4000483e:	48 89 ef             	mov    %rbp,%rdi
    40004841:	e8 aa 0e 00 00       	call   400056f0 <__lockfile>
    40004846:	41 89 c5             	mov    %eax,%r13d
    40004849:	8b 45 00             	mov    0x0(%rbp),%eax
    4000484c:	89 c3                	mov    %eax,%ebx
    4000484e:	83 e0 df             	and    $0xffffffdf,%eax
    40004851:	83 e3 20             	and    $0x20,%ebx
    40004854:	48 83 7d 60 00       	cmpq   $0x0,0x60(%rbp)
    40004859:	89 45 00             	mov    %eax,0x0(%rbp)
    4000485c:	0f 84 9e 00 00 00    	je     40004900 <vfprintf+0x130>
    40004862:	4c 8b 7d 20          	mov    0x20(%rbp),%r15
    40004866:	4d 85 ff             	test   %r15,%r15
    40004869:	0f 84 f3 00 00 00    	je     40004962 <vfprintf+0x192>
    4000486f:	45 31 ff             	xor    %r15d,%r15d
    40004872:	4c 8d 44 24 20       	lea    0x20(%rsp),%r8
    40004877:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
    4000487c:	4c 89 f6             	mov    %r14,%rsi
    4000487f:	48 89 ef             	mov    %rbp,%rdi
    40004882:	48 8d 8c 24 a0 00 00 	lea    0xa0(%rsp),%rcx
    40004889:	00 
    4000488a:	e8 01 f1 ff ff       	call   40003990 <printf_core>
    4000488f:	41 89 c4             	mov    %eax,%r12d
    40004892:	4d 85 ff             	test   %r15,%r15
    40004895:	74 3c                	je     400048d3 <vfprintf+0x103>
    40004897:	31 d2                	xor    %edx,%edx
    40004899:	31 f6                	xor    %esi,%esi
    4000489b:	48 89 ef             	mov    %rbp,%rdi
    4000489e:	ff 55 48             	call   *0x48(%rbp)
    400048a1:	48 83 7d 28 00       	cmpq   $0x0,0x28(%rbp)
    400048a6:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
    400048ab:	4c 89 7d 58          	mov    %r15,0x58(%rbp)
    400048af:	48 c7 45 60 00 00 00 	movq   $0x0,0x60(%rbp)
    400048b6:	00 
    400048b7:	44 0f 44 e0          	cmove  %eax,%r12d
    400048bb:	48 c7 45 20 00 00 00 	movq   $0x0,0x20(%rbp)
    400048c2:	00 
    400048c3:	48 c7 45 38 00 00 00 	movq   $0x0,0x38(%rbp)
    400048ca:	00 
    400048cb:	48 c7 45 28 00 00 00 	movq   $0x0,0x28(%rbp)
    400048d2:	00 
    400048d3:	8b 45 00             	mov    0x0(%rbp),%eax
    400048d6:	ba ff ff ff ff       	mov    $0xffffffff,%edx
    400048db:	a8 20                	test   $0x20,%al
    400048dd:	44 0f 45 e2          	cmovne %edx,%r12d
    400048e1:	09 c3                	or     %eax,%ebx
    400048e3:	89 5d 00             	mov    %ebx,0x0(%rbp)
    400048e6:	45 85 ed             	test   %r13d,%r13d
    400048e9:	75 65                	jne    40004950 <vfprintf+0x180>
    400048eb:	48 81 c4 48 01 00 00 	add    $0x148,%rsp
    400048f2:	44 89 e0             	mov    %r12d,%eax
    400048f5:	5b                   	pop    %rbx
    400048f6:	5d                   	pop    %rbp
    400048f7:	41 5c                	pop    %r12
    400048f9:	41 5d                	pop    %r13
    400048fb:	41 5e                	pop    %r14
    400048fd:	41 5f                	pop    %r15
    400048ff:	c3                   	ret    
    40004900:	48 8d 44 24 50       	lea    0x50(%rsp),%rax
    40004905:	4c 8b 7d 58          	mov    0x58(%rbp),%r15
    40004909:	48 c7 45 60 50 00 00 	movq   $0x50,0x60(%rbp)
    40004910:	00 
    40004911:	48 89 ef             	mov    %rbp,%rdi
    40004914:	48 89 45 58          	mov    %rax,0x58(%rbp)
    40004918:	41 bc ff ff ff ff    	mov    $0xffffffff,%r12d
    4000491e:	48 c7 45 20 00 00 00 	movq   $0x0,0x20(%rbp)
    40004925:	00 
    40004926:	48 c7 45 38 00 00 00 	movq   $0x0,0x38(%rbp)
    4000492d:	00 
    4000492e:	48 c7 45 28 00 00 00 	movq   $0x0,0x28(%rbp)
    40004935:	00 
    40004936:	e8 95 0f 00 00       	call   400058d0 <__towrite>
    4000493b:	85 c0                	test   %eax,%eax
    4000493d:	0f 85 4f ff ff ff    	jne    40004892 <vfprintf+0xc2>
    40004943:	e9 2a ff ff ff       	jmp    40004872 <vfprintf+0xa2>
    40004948:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4000494f:	00 
    40004950:	48 89 ef             	mov    %rbp,%rdi
    40004953:	e8 78 0e 00 00       	call   400057d0 <__unlockfile>
    40004958:	eb 91                	jmp    400048eb <vfprintf+0x11b>
    4000495a:	41 bc ff ff ff ff    	mov    $0xffffffff,%r12d
    40004960:	eb 89                	jmp    400048eb <vfprintf+0x11b>
    40004962:	48 89 ef             	mov    %rbp,%rdi
    40004965:	41 83 cc ff          	or     $0xffffffff,%r12d
    40004969:	e8 62 0f 00 00       	call   400058d0 <__towrite>
    4000496e:	85 c0                	test   %eax,%eax
    40004970:	0f 85 5d ff ff ff    	jne    400048d3 <vfprintf+0x103>
    40004976:	e9 f7 fe ff ff       	jmp    40004872 <vfprintf+0xa2>
    4000497b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040004980 <strnlen>:
    40004980:	55                   	push   %rbp
    40004981:	48 89 f2             	mov    %rsi,%rdx
    40004984:	48 89 fd             	mov    %rdi,%rbp
    40004987:	53                   	push   %rbx
    40004988:	48 89 f3             	mov    %rsi,%rbx
    4000498b:	31 f6                	xor    %esi,%esi
    4000498d:	48 83 ec 08          	sub    $0x8,%rsp
    40004991:	e8 1a 11 00 00       	call   40005ab0 <memchr>
    40004996:	48 89 c2             	mov    %rax,%rdx
    40004999:	48 29 ea             	sub    %rbp,%rdx
    4000499c:	48 85 c0             	test   %rax,%rax
    4000499f:	48 89 d8             	mov    %rbx,%rax
    400049a2:	48 0f 45 c2          	cmovne %rdx,%rax
    400049a6:	48 83 c4 08          	add    $0x8,%rsp
    400049aa:	5b                   	pop    %rbx
    400049ab:	5d                   	pop    %rbp
    400049ac:	c3                   	ret    

00000000400049ad <memcpy>:
    400049ad:	48 89 f8             	mov    %rdi,%rax
    400049b0:	48 83 fa 08          	cmp    $0x8,%rdx
    400049b4:	72 14                	jb     400049ca <memcpy+0x1d>
    400049b6:	f7 c7 07 00 00 00    	test   $0x7,%edi
    400049bc:	74 0c                	je     400049ca <memcpy+0x1d>
    400049be:	a4                   	movsb  %ds:(%rsi),%es:(%rdi)
    400049bf:	48 ff ca             	dec    %rdx
    400049c2:	f7 c7 07 00 00 00    	test   $0x7,%edi
    400049c8:	75 f4                	jne    400049be <memcpy+0x11>
    400049ca:	48 89 d1             	mov    %rdx,%rcx
    400049cd:	48 c1 e9 03          	shr    $0x3,%rcx
    400049d1:	f3 48 a5             	rep movsq %ds:(%rsi),%es:(%rdi)
    400049d4:	83 e2 07             	and    $0x7,%edx
    400049d7:	74 05                	je     400049de <memcpy+0x31>
    400049d9:	a4                   	movsb  %ds:(%rsi),%es:(%rdi)
    400049da:	ff ca                	dec    %edx
    400049dc:	75 fb                	jne    400049d9 <memcpy+0x2c>
    400049de:	c3                   	ret    

00000000400049df <memset>:
    400049df:	48 0f b6 c6          	movzbq %sil,%rax
    400049e3:	49 b8 01 01 01 01 01 	movabs $0x101010101010101,%r8
    400049ea:	01 01 01 
    400049ed:	49 0f af c0          	imul   %r8,%rax
    400049f1:	48 83 fa 7e          	cmp    $0x7e,%rdx
    400049f5:	77 78                	ja     40004a6f <memset+0x90>
    400049f7:	85 d2                	test   %edx,%edx
    400049f9:	74 70                	je     40004a6b <memset+0x8c>
    400049fb:	40 88 37             	mov    %sil,(%rdi)
    400049fe:	40 88 74 17 ff       	mov    %sil,-0x1(%rdi,%rdx,1)
    40004a03:	83 fa 02             	cmp    $0x2,%edx
    40004a06:	76 63                	jbe    40004a6b <memset+0x8c>
    40004a08:	66 89 47 01          	mov    %ax,0x1(%rdi)
    40004a0c:	66 89 44 17 fd       	mov    %ax,-0x3(%rdi,%rdx,1)
    40004a11:	83 fa 06             	cmp    $0x6,%edx
    40004a14:	76 55                	jbe    40004a6b <memset+0x8c>
    40004a16:	89 47 03             	mov    %eax,0x3(%rdi)
    40004a19:	89 44 17 f9          	mov    %eax,-0x7(%rdi,%rdx,1)
    40004a1d:	83 fa 0e             	cmp    $0xe,%edx
    40004a20:	76 49                	jbe    40004a6b <memset+0x8c>
    40004a22:	48 89 47 07          	mov    %rax,0x7(%rdi)
    40004a26:	48 89 44 17 f1       	mov    %rax,-0xf(%rdi,%rdx,1)
    40004a2b:	83 fa 1e             	cmp    $0x1e,%edx
    40004a2e:	76 3b                	jbe    40004a6b <memset+0x8c>
    40004a30:	48 89 47 0f          	mov    %rax,0xf(%rdi)
    40004a34:	48 89 47 17          	mov    %rax,0x17(%rdi)
    40004a38:	48 89 44 17 e1       	mov    %rax,-0x1f(%rdi,%rdx,1)
    40004a3d:	48 89 44 17 e9       	mov    %rax,-0x17(%rdi,%rdx,1)
    40004a42:	83 fa 3e             	cmp    $0x3e,%edx
    40004a45:	76 24                	jbe    40004a6b <memset+0x8c>
    40004a47:	48 89 47 1f          	mov    %rax,0x1f(%rdi)
    40004a4b:	48 89 47 27          	mov    %rax,0x27(%rdi)
    40004a4f:	48 89 47 2f          	mov    %rax,0x2f(%rdi)
    40004a53:	48 89 47 37          	mov    %rax,0x37(%rdi)
    40004a57:	48 89 44 17 c1       	mov    %rax,-0x3f(%rdi,%rdx,1)
    40004a5c:	48 89 44 17 c9       	mov    %rax,-0x37(%rdi,%rdx,1)
    40004a61:	48 89 44 17 d1       	mov    %rax,-0x2f(%rdi,%rdx,1)
    40004a66:	48 89 44 17 d9       	mov    %rax,-0x27(%rdi,%rdx,1)
    40004a6b:	48 89 f8             	mov    %rdi,%rax
    40004a6e:	c3                   	ret    
    40004a6f:	f7 c7 0f 00 00 00    	test   $0xf,%edi
    40004a75:	49 89 f8             	mov    %rdi,%r8
    40004a78:	48 89 44 17 f8       	mov    %rax,-0x8(%rdi,%rdx,1)
    40004a7d:	48 89 d1             	mov    %rdx,%rcx
    40004a80:	75 0b                	jne    40004a8d <memset+0xae>
    40004a82:	48 c1 e9 03          	shr    $0x3,%rcx
    40004a86:	f3 48 ab             	rep stos %rax,%es:(%rdi)
    40004a89:	4c 89 c0             	mov    %r8,%rax
    40004a8c:	c3                   	ret    
    40004a8d:	31 d2                	xor    %edx,%edx
    40004a8f:	29 fa                	sub    %edi,%edx
    40004a91:	83 e2 0f             	and    $0xf,%edx
    40004a94:	48 89 07             	mov    %rax,(%rdi)
    40004a97:	48 89 47 08          	mov    %rax,0x8(%rdi)
    40004a9b:	48 29 d1             	sub    %rdx,%rcx
    40004a9e:	48 01 d7             	add    %rdx,%rdi
    40004aa1:	eb df                	jmp    40004a82 <memset+0xa3>
    40004aa3:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40004aaa:	00 00 00 
    40004aad:	0f 1f 00             	nopl   (%rax)

0000000040004ab0 <cgt_init>:
    40004ab0:	41 54                	push   %r12
    40004ab2:	49 89 f4             	mov    %rsi,%r12
    40004ab5:	be a0 70 00 40       	mov    $0x400070a0,%esi
    40004aba:	55                   	push   %rbp
    40004abb:	89 fd                	mov    %edi,%ebp
    40004abd:	bf b5 70 00 40       	mov    $0x400070b5,%edi
    40004ac2:	48 83 ec 08          	sub    $0x8,%rsp
    40004ac6:	e8 15 06 00 00       	call   400050e0 <__vdsosym>
    40004acb:	48 89 c2             	mov    %rax,%rdx
    40004ace:	b8 b0 4a 00 40       	mov    $0x40004ab0,%eax
    40004ad3:	f0 48 0f b1 15 8c 32 	lock cmpxchg %rdx,0x328c(%rip)        # 40007d68 <vdso_func>
    40004ada:	00 00 
    40004adc:	48 85 d2             	test   %rdx,%rdx
    40004adf:	74 0e                	je     40004aef <cgt_init+0x3f>
    40004ae1:	48 83 c4 08          	add    $0x8,%rsp
    40004ae5:	4c 89 e6             	mov    %r12,%rsi
    40004ae8:	89 ef                	mov    %ebp,%edi
    40004aea:	5d                   	pop    %rbp
    40004aeb:	41 5c                	pop    %r12
    40004aed:	ff e2                	jmp    *%rdx
    40004aef:	48 83 c4 08          	add    $0x8,%rsp
    40004af3:	b8 da ff ff ff       	mov    $0xffffffda,%eax
    40004af8:	5d                   	pop    %rbp
    40004af9:	41 5c                	pop    %r12
    40004afb:	c3                   	ret    
    40004afc:	0f 1f 40 00          	nopl   0x0(%rax)

0000000040004b00 <__clock_gettime>:
    40004b00:	55                   	push   %rbp
    40004b01:	48 89 f5             	mov    %rsi,%rbp
    40004b04:	53                   	push   %rbx
    40004b05:	89 fb                	mov    %edi,%ebx
    40004b07:	48 83 ec 08          	sub    $0x8,%rsp
    40004b0b:	48 8b 05 56 32 00 00 	mov    0x3256(%rip),%rax        # 40007d68 <vdso_func>
    40004b12:	48 85 c0             	test   %rax,%rax
    40004b15:	74 0b                	je     40004b22 <__clock_gettime+0x22>
    40004b17:	ff d0                	call   *%rax
    40004b19:	85 c0                	test   %eax,%eax
    40004b1b:	74 27                	je     40004b44 <__clock_gettime+0x44>
    40004b1d:	83 f8 ea             	cmp    $0xffffffea,%eax
    40004b20:	74 36                	je     40004b58 <__clock_gettime+0x58>
    40004b22:	48 63 fb             	movslq %ebx,%rdi
    40004b25:	b8 e4 00 00 00       	mov    $0xe4,%eax
    40004b2a:	48 89 ee             	mov    %rbp,%rsi
    40004b2d:	0f 05                	syscall 
    40004b2f:	83 f8 da             	cmp    $0xffffffda,%eax
    40004b32:	75 1c                	jne    40004b50 <__clock_gettime+0x50>
    40004b34:	48 c7 c7 ea ff ff ff 	mov    $0xffffffffffffffea,%rdi
    40004b3b:	85 db                	test   %ebx,%ebx
    40004b3d:	74 31                	je     40004b70 <__clock_gettime+0x70>
    40004b3f:	e8 fc d3 ff ff       	call   40001f40 <__syscall_ret>
    40004b44:	48 83 c4 08          	add    $0x8,%rsp
    40004b48:	5b                   	pop    %rbx
    40004b49:	5d                   	pop    %rbp
    40004b4a:	c3                   	ret    
    40004b4b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    40004b50:	48 63 f8             	movslq %eax,%rdi
    40004b53:	eb ea                	jmp    40004b3f <__clock_gettime+0x3f>
    40004b55:	0f 1f 00             	nopl   (%rax)
    40004b58:	48 c7 c7 ea ff ff ff 	mov    $0xffffffffffffffea,%rdi
    40004b5f:	e8 dc d3 ff ff       	call   40001f40 <__syscall_ret>
    40004b64:	eb de                	jmp    40004b44 <__clock_gettime+0x44>
    40004b66:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40004b6d:	00 00 00 
    40004b70:	b8 60 00 00 00       	mov    $0x60,%eax
    40004b75:	31 f6                	xor    %esi,%esi
    40004b77:	48 89 ef             	mov    %rbp,%rdi
    40004b7a:	0f 05                	syscall 
    40004b7c:	69 45 08 e8 03 00 00 	imul   $0x3e8,0x8(%rbp),%eax
    40004b83:	48 98                	cltq   
    40004b85:	48 89 45 08          	mov    %rax,0x8(%rbp)
    40004b89:	48 83 c4 08          	add    $0x8,%rsp
    40004b8d:	31 c0                	xor    %eax,%eax
    40004b8f:	5b                   	pop    %rbx
    40004b90:	5d                   	pop    %rbp
    40004b91:	c3                   	ret    
    40004b92:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40004b99:	00 00 00 
    40004b9c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000040004ba0 <nanosleep>:
    40004ba0:	48 83 ec 08          	sub    $0x8,%rsp
    40004ba4:	48 89 fa             	mov    %rdi,%rdx
    40004ba7:	48 89 f1             	mov    %rsi,%rcx
    40004baa:	31 ff                	xor    %edi,%edi
    40004bac:	31 f6                	xor    %esi,%esi
    40004bae:	e8 1d 10 00 00       	call   40005bd0 <__clock_nanosleep>
    40004bb3:	f7 d8                	neg    %eax
    40004bb5:	48 63 f8             	movslq %eax,%rdi
    40004bb8:	e8 83 d3 ff ff       	call   40001f40 <__syscall_ret>
    40004bbd:	48 83 c4 08          	add    $0x8,%rsp
    40004bc1:	c3                   	ret    
    40004bc2:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40004bc9:	00 00 00 
    40004bcc:	0f 1f 40 00          	nopl   0x0(%rax)

0000000040004bd0 <__aio_close>:
    40004bd0:	89 f8                	mov    %edi,%eax
    40004bd2:	c3                   	ret    
    40004bd3:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40004bda:	00 00 00 
    40004bdd:	0f 1f 00             	nopl   (%rax)

0000000040004be0 <close>:
    40004be0:	48 83 ec 08          	sub    $0x8,%rsp
    40004be4:	e8 e7 ff ff ff       	call   40004bd0 <__aio_close>
    40004be9:	48 83 ec 08          	sub    $0x8,%rsp
    40004bed:	45 31 c9             	xor    %r9d,%r9d
    40004bf0:	45 31 c0             	xor    %r8d,%r8d
    40004bf3:	6a 00                	push   $0x0
    40004bf5:	31 c9                	xor    %ecx,%ecx
    40004bf7:	31 d2                	xor    %edx,%edx
    40004bf9:	48 63 f0             	movslq %eax,%rsi
    40004bfc:	bf 03 00 00 00       	mov    $0x3,%edi
    40004c01:	e8 b4 c9 ff ff       	call   400015ba <__syscall_cp>
    40004c06:	5a                   	pop    %rdx
    40004c07:	31 d2                	xor    %edx,%edx
    40004c09:	59                   	pop    %rcx
    40004c0a:	83 f8 fc             	cmp    $0xfffffffc,%eax
    40004c0d:	0f 44 c2             	cmove  %edx,%eax
    40004c10:	48 63 f8             	movslq %eax,%rdi
    40004c13:	e8 28 d3 ff ff       	call   40001f40 <__syscall_ret>
    40004c18:	48 83 c4 08          	add    $0x8,%rsp
    40004c1c:	c3                   	ret    
    40004c1d:	0f 1f 00             	nopl   (%rax)

0000000040004c20 <dup>:
    40004c20:	48 83 ec 08          	sub    $0x8,%rsp
    40004c24:	48 63 ff             	movslq %edi,%rdi
    40004c27:	b8 20 00 00 00       	mov    $0x20,%eax
    40004c2c:	0f 05                	syscall 
    40004c2e:	48 89 c7             	mov    %rax,%rdi
    40004c31:	e8 0a d3 ff ff       	call   40001f40 <__syscall_ret>
    40004c36:	48 83 c4 08          	add    $0x8,%rsp
    40004c3a:	c3                   	ret    
    40004c3b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040004c40 <dup2>:
    40004c40:	48 83 ec 08          	sub    $0x8,%rsp
    40004c44:	48 63 f6             	movslq %esi,%rsi
    40004c47:	48 63 ff             	movslq %edi,%rdi
    40004c4a:	41 b8 21 00 00 00    	mov    $0x21,%r8d
    40004c50:	4c 89 c0             	mov    %r8,%rax
    40004c53:	0f 05                	syscall 
    40004c55:	83 f8 f0             	cmp    $0xfffffff0,%eax
    40004c58:	74 f6                	je     40004c50 <dup2+0x10>
    40004c5a:	48 63 f8             	movslq %eax,%rdi
    40004c5d:	e8 de d2 ff ff       	call   40001f40 <__syscall_ret>
    40004c62:	48 83 c4 08          	add    $0x8,%rsp
    40004c66:	c3                   	ret    
    40004c67:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    40004c6e:	00 00 

0000000040004c70 <ftruncate>:
    40004c70:	48 83 ec 08          	sub    $0x8,%rsp
    40004c74:	48 63 ff             	movslq %edi,%rdi
    40004c77:	b8 4d 00 00 00       	mov    $0x4d,%eax
    40004c7c:	0f 05                	syscall 
    40004c7e:	48 89 c7             	mov    %rax,%rdi
    40004c81:	e8 ba d2 ff ff       	call   40001f40 <__syscall_ret>
    40004c86:	48 83 c4 08          	add    $0x8,%rsp
    40004c8a:	c3                   	ret    
    40004c8b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040004c90 <__lseek>:
    40004c90:	48 63 ff             	movslq %edi,%rdi
    40004c93:	48 63 d2             	movslq %edx,%rdx
    40004c96:	b8 08 00 00 00       	mov    $0x8,%eax
    40004c9b:	0f 05                	syscall 
    40004c9d:	48 89 c7             	mov    %rax,%rdi
    40004ca0:	e9 9b d2 ff ff       	jmp    40001f40 <__syscall_ret>
    40004ca5:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40004cac:	00 00 00 
    40004caf:	90                   	nop

0000000040004cb0 <pipe>:
    40004cb0:	48 83 ec 08          	sub    $0x8,%rsp
    40004cb4:	b8 16 00 00 00       	mov    $0x16,%eax
    40004cb9:	0f 05                	syscall 
    40004cbb:	48 89 c7             	mov    %rax,%rdi
    40004cbe:	e8 7d d2 ff ff       	call   40001f40 <__syscall_ret>
    40004cc3:	48 83 c4 08          	add    $0x8,%rsp
    40004cc7:	c3                   	ret    
    40004cc8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    40004ccf:	00 

0000000040004cd0 <read>:
    40004cd0:	48 83 ec 10          	sub    $0x10,%rsp
    40004cd4:	49 89 f2             	mov    %rsi,%r10
    40004cd7:	48 89 d1             	mov    %rdx,%rcx
    40004cda:	48 63 f7             	movslq %edi,%rsi
    40004cdd:	6a 00                	push   $0x0
    40004cdf:	31 ff                	xor    %edi,%edi
    40004ce1:	45 31 c9             	xor    %r9d,%r9d
    40004ce4:	45 31 c0             	xor    %r8d,%r8d
    40004ce7:	4c 89 d2             	mov    %r10,%rdx
    40004cea:	e8 cb c8 ff ff       	call   400015ba <__syscall_cp>
    40004cef:	48 83 c4 18          	add    $0x18,%rsp
    40004cf3:	48 89 c7             	mov    %rax,%rdi
    40004cf6:	e9 45 d2 ff ff       	jmp    40001f40 <__syscall_ret>
    40004cfb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040004d00 <write>:
    40004d00:	48 83 ec 10          	sub    $0x10,%rsp
    40004d04:	49 89 f2             	mov    %rsi,%r10
    40004d07:	48 89 d1             	mov    %rdx,%rcx
    40004d0a:	48 63 f7             	movslq %edi,%rsi
    40004d0d:	6a 00                	push   $0x0
    40004d0f:	bf 01 00 00 00       	mov    $0x1,%edi
    40004d14:	45 31 c9             	xor    %r9d,%r9d
    40004d17:	45 31 c0             	xor    %r8d,%r8d
    40004d1a:	4c 89 d2             	mov    %r10,%rdx
    40004d1d:	e8 98 c8 ff ff       	call   400015ba <__syscall_cp>
    40004d22:	48 83 c4 18          	add    $0x18,%rsp
    40004d26:	48 89 c7             	mov    %rax,%rdi
    40004d29:	e9 12 d2 ff ff       	jmp    40001f40 <__syscall_ret>
    40004d2e:	66 90                	xchg   %ax,%ax

0000000040004d30 <__init_tp>:
    40004d30:	53                   	push   %rbx
    40004d31:	48 89 fb             	mov    %rdi,%rbx
    40004d34:	48 89 3f             	mov    %rdi,(%rdi)
    40004d37:	e8 82 0e 00 00       	call   40005bbe <__set_thread_area>
    40004d3c:	85 c0                	test   %eax,%eax
    40004d3e:	78 4f                	js     40004d8f <__init_tp+0x5f>
    40004d40:	75 07                	jne    40004d49 <__init_tp+0x19>
    40004d42:	c6 05 57 30 00 00 01 	movb   $0x1,0x3057(%rip)        # 40007da0 <__libc>
    40004d49:	c7 43 38 02 00 00 00 	movl   $0x2,0x38(%rbx)
    40004d50:	b8 da 00 00 00       	mov    $0xda,%eax
    40004d55:	bf d0 83 00 40       	mov    $0x400083d0,%edi
    40004d5a:	0f 05                	syscall 
    40004d5c:	89 43 30             	mov    %eax,0x30(%rbx)
    40004d5f:	48 8d 83 88 00 00 00 	lea    0x88(%rbx),%rax
    40004d66:	48 89 83 88 00 00 00 	mov    %rax,0x88(%rbx)
    40004d6d:	48 8b 05 0c 30 00 00 	mov    0x300c(%rip),%rax        # 40007d80 <__sysinfo>
    40004d74:	48 c7 83 a8 00 00 00 	movq   $0x40007dd8,0xa8(%rbx)
    40004d7b:	d8 7d 00 40 
    40004d7f:	48 89 43 20          	mov    %rax,0x20(%rbx)
    40004d83:	31 c0                	xor    %eax,%eax
    40004d85:	48 89 5b 10          	mov    %rbx,0x10(%rbx)
    40004d89:	48 89 5b 18          	mov    %rbx,0x18(%rbx)
    40004d8d:	5b                   	pop    %rbx
    40004d8e:	c3                   	ret    
    40004d8f:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
    40004d94:	5b                   	pop    %rbx
    40004d95:	c3                   	ret    
    40004d96:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40004d9d:	00 00 00 

0000000040004da0 <__copy_tls>:
    40004da0:	41 55                	push   %r13
    40004da2:	49 89 fd             	mov    %rdi,%r13
    40004da5:	41 54                	push   %r12
    40004da7:	55                   	push   %rbp
    40004da8:	53                   	push   %rbx
    40004da9:	48 83 ec 08          	sub    $0x8,%rsp
    40004dad:	48 8b 05 04 30 00 00 	mov    0x3004(%rip),%rax        # 40007db8 <__libc+0x18>
    40004db4:	4c 8b 25 05 30 00 00 	mov    0x3005(%rip),%r12        # 40007dc0 <__libc+0x20>
    40004dbb:	48 8b 1d ee 2f 00 00 	mov    0x2fee(%rip),%rbx        # 40007db0 <__libc+0x10>
    40004dc2:	48 8d 84 07 38 ff ff 	lea    -0xc8(%rdi,%rax,1),%rax
    40004dc9:	ff 
    40004dca:	49 f7 dc             	neg    %r12
    40004dcd:	49 21 c4             	and    %rax,%r12
    40004dd0:	48 85 db             	test   %rbx,%rbx
    40004dd3:	74 36                	je     40004e0b <__copy_tls+0x6b>
    40004dd5:	48 8d 6f 08          	lea    0x8(%rdi),%rbp
    40004dd9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40004de0:	4c 89 e0             	mov    %r12,%rax
    40004de3:	48 2b 43 28          	sub    0x28(%rbx),%rax
    40004de7:	4c 89 e7             	mov    %r12,%rdi
    40004dea:	48 83 c5 08          	add    $0x8,%rbp
    40004dee:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    40004df2:	48 8b 53 10          	mov    0x10(%rbx),%rdx
    40004df6:	48 8b 73 08          	mov    0x8(%rbx),%rsi
    40004dfa:	48 2b 7b 28          	sub    0x28(%rbx),%rdi
    40004dfe:	e8 aa fb ff ff       	call   400049ad <memcpy>
    40004e03:	48 8b 1b             	mov    (%rbx),%rbx
    40004e06:	48 85 db             	test   %rbx,%rbx
    40004e09:	75 d5                	jne    40004de0 <__copy_tls+0x40>
    40004e0b:	48 8b 05 b6 2f 00 00 	mov    0x2fb6(%rip),%rax        # 40007dc8 <__libc+0x28>
    40004e12:	49 89 45 00          	mov    %rax,0x0(%r13)
    40004e16:	4c 89 e0             	mov    %r12,%rax
    40004e19:	4d 89 6c 24 08       	mov    %r13,0x8(%r12)
    40004e1e:	48 83 c4 08          	add    $0x8,%rsp
    40004e22:	5b                   	pop    %rbx
    40004e23:	5d                   	pop    %rbp
    40004e24:	41 5c                	pop    %r12
    40004e26:	41 5d                	pop    %r13
    40004e28:	c3                   	ret    
    40004e29:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000040004e30 <__init_tls>:
    40004e30:	53                   	push   %rbx
    40004e31:	48 8b 4f 28          	mov    0x28(%rdi),%rcx
    40004e35:	4c 8b 5f 18          	mov    0x18(%rdi),%r11
    40004e39:	48 85 c9             	test   %rcx,%rcx
    40004e3c:	0f 84 de 01 00 00    	je     40005020 <__init_tls+0x1f0>
    40004e42:	48 8b 7f 20          	mov    0x20(%rdi),%rdi
    40004e46:	4c 89 d8             	mov    %r11,%rax
    40004e49:	31 f6                	xor    %esi,%esi
    40004e4b:	45 31 c0             	xor    %r8d,%r8d
    40004e4e:	41 b9 00 00 00 00    	mov    $0x0,%r9d
    40004e54:	eb 28                	jmp    40004e7e <__init_tls+0x4e>
    40004e56:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40004e5d:	00 00 00 
    40004e60:	83 fa 02             	cmp    $0x2,%edx
    40004e63:	0f 84 97 01 00 00    	je     40005000 <__init_tls+0x1d0>
    40004e69:	83 fa 07             	cmp    $0x7,%edx
    40004e6c:	0f 85 53 01 00 00    	jne    40004fc5 <__init_tls+0x195>
    40004e72:	49 89 c0             	mov    %rax,%r8
    40004e75:	48 01 f8             	add    %rdi,%rax
    40004e78:	48 83 e9 01          	sub    $0x1,%rcx
    40004e7c:	74 17                	je     40004e95 <__init_tls+0x65>
    40004e7e:	8b 10                	mov    (%rax),%edx
    40004e80:	83 fa 06             	cmp    $0x6,%edx
    40004e83:	75 db                	jne    40004e60 <__init_tls+0x30>
    40004e85:	4c 89 de             	mov    %r11,%rsi
    40004e88:	48 2b 70 10          	sub    0x10(%rax),%rsi
    40004e8c:	48 01 f8             	add    %rdi,%rax
    40004e8f:	48 83 e9 01          	sub    $0x1,%rcx
    40004e93:	75 e9                	jne    40004e7e <__init_tls+0x4e>
    40004e95:	4d 85 c0             	test   %r8,%r8
    40004e98:	0f 84 82 01 00 00    	je     40005020 <__init_tls+0x1f0>
    40004e9e:	49 8b 40 20          	mov    0x20(%r8),%rax
    40004ea2:	49 03 70 10          	add    0x10(%r8),%rsi
    40004ea6:	48 c7 05 ff 2e 00 00 	movq   $0x40008240,0x2eff(%rip)        # 40007db0 <__libc+0x10>
    40004ead:	40 82 00 40 
    40004eb1:	49 8b 50 28          	mov    0x28(%r8),%rdx
    40004eb5:	48 89 35 8c 33 00 00 	mov    %rsi,0x338c(%rip)        # 40008248 <main_tls+0x8>
    40004ebc:	48 89 05 8d 33 00 00 	mov    %rax,0x338d(%rip)        # 40008250 <main_tls+0x10>
    40004ec3:	49 8b 40 30          	mov    0x30(%r8),%rax
    40004ec7:	48 c7 05 f6 2e 00 00 	movq   $0x1,0x2ef6(%rip)        # 40007dc8 <__libc+0x28>
    40004ece:	01 00 00 00 
    40004ed2:	48 89 05 87 33 00 00 	mov    %rax,0x3387(%rip)        # 40008260 <main_tls+0x20>
    40004ed9:	48 01 d6             	add    %rdx,%rsi
    40004edc:	48 8d 48 ff          	lea    -0x1(%rax),%rcx
    40004ee0:	48 f7 de             	neg    %rsi
    40004ee3:	48 21 ce             	and    %rcx,%rsi
    40004ee6:	48 01 d6             	add    %rdx,%rsi
    40004ee9:	48 8d 90 df 00 00 00 	lea    0xdf(%rax),%rdx
    40004ef0:	48 89 35 61 33 00 00 	mov    %rsi,0x3361(%rip)        # 40008258 <main_tls+0x18>
    40004ef7:	48 89 35 6a 33 00 00 	mov    %rsi,0x336a(%rip)        # 40008268 <main_tls+0x28>
    40004efe:	48 83 f8 07          	cmp    $0x7,%rax
    40004f02:	77 15                	ja     40004f19 <__init_tls+0xe9>
    40004f04:	48 c7 05 51 33 00 00 	movq   $0x8,0x3351(%rip)        # 40008260 <main_tls+0x20>
    40004f0b:	08 00 00 00 
    40004f0f:	ba e7 00 00 00       	mov    $0xe7,%edx
    40004f14:	b8 08 00 00 00       	mov    $0x8,%eax
    40004f19:	48 01 d6             	add    %rdx,%rsi
    40004f1c:	48 89 05 9d 2e 00 00 	mov    %rax,0x2e9d(%rip)        # 40007dc0 <__libc+0x20>
    40004f23:	bf 80 82 00 40       	mov    $0x40008280,%edi
    40004f28:	48 83 e6 f8          	and    $0xfffffffffffffff8,%rsi
    40004f2c:	48 89 35 85 2e 00 00 	mov    %rsi,0x2e85(%rip)        # 40007db8 <__libc+0x18>
    40004f33:	48 81 fe 50 01 00 00 	cmp    $0x150,%rsi
    40004f3a:	76 21                	jbe    40004f5d <__init_tls+0x12d>
    40004f3c:	41 ba 22 00 00 00    	mov    $0x22,%r10d
    40004f42:	45 31 c9             	xor    %r9d,%r9d
    40004f45:	b8 09 00 00 00       	mov    $0x9,%eax
    40004f4a:	31 ff                	xor    %edi,%edi
    40004f4c:	49 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%r8
    40004f53:	ba 03 00 00 00       	mov    $0x3,%edx
    40004f58:	0f 05                	syscall 
    40004f5a:	48 89 c7             	mov    %rax,%rdi
    40004f5d:	e8 3e fe ff ff       	call   40004da0 <__copy_tls>
    40004f62:	48 89 00             	mov    %rax,(%rax)
    40004f65:	48 89 c7             	mov    %rax,%rdi
    40004f68:	48 89 c3             	mov    %rax,%rbx
    40004f6b:	e8 4e 0c 00 00       	call   40005bbe <__set_thread_area>
    40004f70:	85 c0                	test   %eax,%eax
    40004f72:	0f 88 9d 00 00 00    	js     40005015 <__init_tls+0x1e5>
    40004f78:	75 07                	jne    40004f81 <__init_tls+0x151>
    40004f7a:	c6 05 1f 2e 00 00 01 	movb   $0x1,0x2e1f(%rip)        # 40007da0 <__libc>
    40004f81:	c7 43 38 02 00 00 00 	movl   $0x2,0x38(%rbx)
    40004f88:	b8 da 00 00 00       	mov    $0xda,%eax
    40004f8d:	bf d0 83 00 40       	mov    $0x400083d0,%edi
    40004f92:	0f 05                	syscall 
    40004f94:	89 43 30             	mov    %eax,0x30(%rbx)
    40004f97:	48 8d 83 88 00 00 00 	lea    0x88(%rbx),%rax
    40004f9e:	48 89 83 88 00 00 00 	mov    %rax,0x88(%rbx)
    40004fa5:	48 8b 05 d4 2d 00 00 	mov    0x2dd4(%rip),%rax        # 40007d80 <__sysinfo>
    40004fac:	48 c7 83 a8 00 00 00 	movq   $0x40007dd8,0xa8(%rbx)
    40004fb3:	d8 7d 00 40 
    40004fb7:	48 89 43 20          	mov    %rax,0x20(%rbx)
    40004fbb:	48 89 5b 10          	mov    %rbx,0x10(%rbx)
    40004fbf:	48 89 5b 18          	mov    %rbx,0x18(%rbx)
    40004fc3:	5b                   	pop    %rbx
    40004fc4:	c3                   	ret    
    40004fc5:	81 fa 51 e5 74 64    	cmp    $0x6474e551,%edx
    40004fcb:	0f 85 a4 fe ff ff    	jne    40004e75 <__init_tls+0x45>
    40004fd1:	48 8b 50 28          	mov    0x28(%rax),%rdx
    40004fd5:	44 8b 15 98 2d 00 00 	mov    0x2d98(%rip),%r10d        # 40007d74 <__default_stacksize>
    40004fdc:	4c 39 d2             	cmp    %r10,%rdx
    40004fdf:	0f 86 90 fe ff ff    	jbe    40004e75 <__init_tls+0x45>
    40004fe5:	41 ba 00 00 80 00    	mov    $0x800000,%r10d
    40004feb:	4c 39 d2             	cmp    %r10,%rdx
    40004fee:	49 0f 47 d2          	cmova  %r10,%rdx
    40004ff2:	89 15 7c 2d 00 00    	mov    %edx,0x2d7c(%rip)        # 40007d74 <__default_stacksize>
    40004ff8:	e9 78 fe ff ff       	jmp    40004e75 <__init_tls+0x45>
    40004ffd:	0f 1f 00             	nopl   (%rax)
    40005000:	4d 85 c9             	test   %r9,%r9
    40005003:	0f 84 6c fe ff ff    	je     40004e75 <__init_tls+0x45>
    40005009:	4c 89 ce             	mov    %r9,%rsi
    4000500c:	48 2b 70 10          	sub    0x10(%rax),%rsi
    40005010:	e9 60 fe ff ff       	jmp    40004e75 <__init_tls+0x45>
    40005015:	f4                   	hlt    
    40005016:	5b                   	pop    %rbx
    40005017:	c3                   	ret    
    40005018:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4000501f:	00 
    40005020:	48 8b 15 31 32 00 00 	mov    0x3231(%rip),%rdx        # 40008258 <main_tls+0x18>
    40005027:	48 8b 35 1a 32 00 00 	mov    0x321a(%rip),%rsi        # 40008248 <main_tls+0x8>
    4000502e:	48 8b 05 2b 32 00 00 	mov    0x322b(%rip),%rax        # 40008260 <main_tls+0x20>
    40005035:	e9 9f fe ff ff       	jmp    40004ed9 <__init_tls+0xa9>
    4000503a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

0000000040005040 <__errno_location>:
    40005040:	64 48 8b 04 25 00 00 	mov    %fs:0x0,%rax
    40005047:	00 00 
    40005049:	48 83 c0 34          	add    $0x34,%rax
    4000504d:	c3                   	ret    
    4000504e:	66 90                	xchg   %ax,%ax

0000000040005050 <__strerror_l>:
    40005050:	41 b8 e0 71 00 40    	mov    $0x400071e0,%r8d
    40005056:	81 ff 83 00 00 00    	cmp    $0x83,%edi
    4000505c:	77 13                	ja     40005071 <__strerror_l+0x21>
    4000505e:	48 63 ff             	movslq %edi,%rdi
    40005061:	44 0f b7 84 3f c0 70 	movzwl 0x400070c0(%rdi,%rdi,1),%r8d
    40005068:	00 40 
    4000506a:	49 81 c0 e0 71 00 40 	add    $0x400071e0,%r8
    40005071:	48 8b 76 28          	mov    0x28(%rsi),%rsi
    40005075:	4c 89 c7             	mov    %r8,%rdi
    40005078:	e9 33 03 00 00       	jmp    400053b0 <__lctrans>
    4000507d:	0f 1f 00             	nopl   (%rax)

0000000040005080 <strerror>:
    40005080:	64 48 8b 04 25 00 00 	mov    %fs:0x0,%rax
    40005087:	00 00 
    40005089:	41 b8 e0 71 00 40    	mov    $0x400071e0,%r8d
    4000508f:	48 8b 80 a8 00 00 00 	mov    0xa8(%rax),%rax
    40005096:	81 ff 83 00 00 00    	cmp    $0x83,%edi
    4000509c:	77 13                	ja     400050b1 <strerror+0x31>
    4000509e:	48 63 ff             	movslq %edi,%rdi
    400050a1:	44 0f b7 84 3f c0 70 	movzwl 0x400070c0(%rdi,%rdi,1),%r8d
    400050a8:	00 40 
    400050aa:	49 81 c0 e0 71 00 40 	add    $0x400071e0,%r8
    400050b1:	48 8b 70 28          	mov    0x28(%rax),%rsi
    400050b5:	4c 89 c7             	mov    %r8,%rdi
    400050b8:	e9 f3 02 00 00       	jmp    400053b0 <__lctrans>
    400050bd:	0f 1f 00             	nopl   (%rax)

00000000400050c0 <_Exit>:
    400050c0:	48 63 ff             	movslq %edi,%rdi
    400050c3:	b8 e7 00 00 00       	mov    $0xe7,%eax
    400050c8:	0f 05                	syscall 
    400050ca:	ba 3c 00 00 00       	mov    $0x3c,%edx
    400050cf:	90                   	nop
    400050d0:	48 89 d0             	mov    %rdx,%rax
    400050d3:	0f 05                	syscall 
    400050d5:	eb f9                	jmp    400050d0 <_Exit+0x10>
    400050d7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    400050de:	00 00 

00000000400050e0 <__vdsosym>:
    400050e0:	41 57                	push   %r15
    400050e2:	41 56                	push   %r14
    400050e4:	41 55                	push   %r13
    400050e6:	41 54                	push   %r12
    400050e8:	49 89 fc             	mov    %rdi,%r12
    400050eb:	55                   	push   %rbp
    400050ec:	48 89 f5             	mov    %rsi,%rbp
    400050ef:	53                   	push   %rbx
    400050f0:	48 83 ec 28          	sub    $0x28,%rsp
    400050f4:	48 8b 35 ad 2c 00 00 	mov    0x2cad(%rip),%rsi        # 40007da8 <__libc+0x8>
    400050fb:	48 8b 06             	mov    (%rsi),%rax
    400050fe:	48 83 f8 21          	cmp    $0x21,%rax
    40005102:	0f 84 89 02 00 00    	je     40005391 <__vdsosym+0x2b1>
    40005108:	31 c9                	xor    %ecx,%ecx
    4000510a:	eb 15                	jmp    40005121 <__vdsosym+0x41>
    4000510c:	0f 1f 40 00          	nopl   0x0(%rax)
    40005110:	48 8d 51 02          	lea    0x2(%rcx),%rdx
    40005114:	48 8b 04 d6          	mov    (%rsi,%rdx,8),%rax
    40005118:	48 83 f8 21          	cmp    $0x21,%rax
    4000511c:	74 22                	je     40005140 <__vdsosym+0x60>
    4000511e:	48 89 d1             	mov    %rdx,%rcx
    40005121:	48 85 c0             	test   %rax,%rax
    40005124:	75 ea                	jne    40005110 <__vdsosym+0x30>
    40005126:	31 c0                	xor    %eax,%eax
    40005128:	48 83 c4 28          	add    $0x28,%rsp
    4000512c:	5b                   	pop    %rbx
    4000512d:	5d                   	pop    %rbp
    4000512e:	41 5c                	pop    %r12
    40005130:	41 5d                	pop    %r13
    40005132:	41 5e                	pop    %r14
    40005134:	41 5f                	pop    %r15
    40005136:	c3                   	ret    
    40005137:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    4000513e:	00 00 
    40005140:	48 8d 04 cd 18 00 00 	lea    0x18(,%rcx,8),%rax
    40005147:	00 
    40005148:	48 8b 3c 06          	mov    (%rsi,%rax,1),%rdi
    4000514c:	48 85 ff             	test   %rdi,%rdi
    4000514f:	74 d5                	je     40005126 <__vdsosym+0x46>
    40005151:	48 8b 47 20          	mov    0x20(%rdi),%rax
    40005155:	44 0f b7 47 38       	movzwl 0x38(%rdi),%r8d
    4000515a:	48 01 f8             	add    %rdi,%rax
    4000515d:	4d 85 c0             	test   %r8,%r8
    40005160:	74 c4                	je     40005126 <__vdsosym+0x46>
    40005162:	44 0f b7 4f 36       	movzwl 0x36(%rdi),%r9d
    40005167:	49 c7 c5 ff ff ff ff 	mov    $0xffffffffffffffff,%r13
    4000516e:	31 d2                	xor    %edx,%edx
    40005170:	31 c9                	xor    %ecx,%ecx
    40005172:	eb 1c                	jmp    40005190 <__vdsosym+0xb0>
    40005174:	0f 1f 40 00          	nopl   0x0(%rax)
    40005178:	83 fe 02             	cmp    $0x2,%esi
    4000517b:	75 07                	jne    40005184 <__vdsosym+0xa4>
    4000517d:	48 8b 50 08          	mov    0x8(%rax),%rdx
    40005181:	48 01 fa             	add    %rdi,%rdx
    40005184:	48 83 c1 01          	add    $0x1,%rcx
    40005188:	4c 01 c8             	add    %r9,%rax
    4000518b:	4c 39 c1             	cmp    %r8,%rcx
    4000518e:	74 1e                	je     400051ae <__vdsosym+0xce>
    40005190:	8b 30                	mov    (%rax),%esi
    40005192:	83 fe 01             	cmp    $0x1,%esi
    40005195:	75 e1                	jne    40005178 <__vdsosym+0x98>
    40005197:	4c 8b 68 08          	mov    0x8(%rax),%r13
    4000519b:	48 83 c1 01          	add    $0x1,%rcx
    4000519f:	49 01 fd             	add    %rdi,%r13
    400051a2:	4c 2b 68 10          	sub    0x10(%rax),%r13
    400051a6:	4c 01 c8             	add    %r9,%rax
    400051a9:	4c 39 c1             	cmp    %r8,%rcx
    400051ac:	75 e2                	jne    40005190 <__vdsosym+0xb0>
    400051ae:	48 85 d2             	test   %rdx,%rdx
    400051b1:	0f 84 6f ff ff ff    	je     40005126 <__vdsosym+0x46>
    400051b7:	49 83 fd ff          	cmp    $0xffffffffffffffff,%r13
    400051bb:	0f 84 65 ff ff ff    	je     40005126 <__vdsosym+0x46>
    400051c1:	48 8b 1a             	mov    (%rdx),%rbx
    400051c4:	48 85 db             	test   %rbx,%rbx
    400051c7:	0f 84 59 ff ff ff    	je     40005126 <__vdsosym+0x46>
    400051cd:	48 8d 42 08          	lea    0x8(%rdx),%rax
    400051d1:	45 31 ff             	xor    %r15d,%r15d
    400051d4:	45 31 c0             	xor    %r8d,%r8d
    400051d7:	31 c9                	xor    %ecx,%ecx
    400051d9:	31 f6                	xor    %esi,%esi
    400051db:	45 31 f6             	xor    %r14d,%r14d
    400051de:	eb 20                	jmp    40005200 <__vdsosym+0x120>
    400051e0:	48 83 fb 04          	cmp    $0x4,%rbx
    400051e4:	0f 84 36 01 00 00    	je     40005320 <__vdsosym+0x240>
    400051ea:	48 83 fb 05          	cmp    $0x5,%rbx
    400051ee:	4c 0f 44 f2          	cmove  %rdx,%r14
    400051f2:	48 8b 58 08          	mov    0x8(%rax),%rbx
    400051f6:	48 83 c0 10          	add    $0x10,%rax
    400051fa:	48 85 db             	test   %rbx,%rbx
    400051fd:	74 38                	je     40005237 <__vdsosym+0x157>
    400051ff:	90                   	nop
    40005200:	48 8b 10             	mov    (%rax),%rdx
    40005203:	4c 01 ea             	add    %r13,%rdx
    40005206:	48 83 fb 06          	cmp    $0x6,%rbx
    4000520a:	0f 84 30 01 00 00    	je     40005340 <__vdsosym+0x260>
    40005210:	76 ce                	jbe    400051e0 <__vdsosym+0x100>
    40005212:	48 81 fb f0 ff ff 6f 	cmp    $0x6ffffff0,%rbx
    40005219:	0f 84 11 01 00 00    	je     40005330 <__vdsosym+0x250>
    4000521f:	48 81 fb fc ff ff 6f 	cmp    $0x6ffffffc,%rbx
    40005226:	4c 0f 44 fa          	cmove  %rdx,%r15
    4000522a:	48 8b 58 08          	mov    0x8(%rax),%rbx
    4000522e:	48 83 c0 10          	add    $0x10,%rax
    40005232:	48 85 db             	test   %rbx,%rbx
    40005235:	75 c9                	jne    40005200 <__vdsosym+0x120>
    40005237:	4d 85 f6             	test   %r14,%r14
    4000523a:	0f 94 c0             	sete   %al
    4000523d:	48 85 f6             	test   %rsi,%rsi
    40005240:	0f 94 c2             	sete   %dl
    40005243:	08 d0                	or     %dl,%al
    40005245:	0f 85 db fe ff ff    	jne    40005126 <__vdsosym+0x46>
    4000524b:	48 85 c9             	test   %rcx,%rcx
    4000524e:	0f 84 d2 fe ff ff    	je     40005126 <__vdsosym+0x46>
    40005254:	8b 41 04             	mov    0x4(%rcx),%eax
    40005257:	4d 85 ff             	test   %r15,%r15
    4000525a:	4c 0f 44 c3          	cmove  %rbx,%r8
    4000525e:	85 c0                	test   %eax,%eax
    40005260:	0f 84 c0 fe ff ff    	je     40005126 <__vdsosym+0x46>
    40005266:	48 89 f2             	mov    %rsi,%rdx
    40005269:	41 b9 27 00 00 00    	mov    $0x27,%r9d
    4000526f:	90                   	nop
    40005270:	0f b6 42 04          	movzbl 0x4(%rdx),%eax
    40005274:	89 c6                	mov    %eax,%esi
    40005276:	83 e6 0f             	and    $0xf,%esi
    40005279:	41 0f a3 f1          	bt     %esi,%r9d
    4000527d:	0f 83 7d 00 00 00    	jae    40005300 <__vdsosym+0x220>
    40005283:	c0 e8 04             	shr    $0x4,%al
    40005286:	be 06 04 00 00       	mov    $0x406,%esi
    4000528b:	0f a3 c6             	bt     %eax,%esi
    4000528e:	73 70                	jae    40005300 <__vdsosym+0x220>
    40005290:	66 83 7a 06 00       	cmpw   $0x0,0x6(%rdx)
    40005295:	74 69                	je     40005300 <__vdsosym+0x220>
    40005297:	8b 32                	mov    (%rdx),%esi
    40005299:	48 89 ef             	mov    %rbp,%rdi
    4000529c:	4c 89 44 24 18       	mov    %r8,0x18(%rsp)
    400052a1:	48 89 4c 24 10       	mov    %rcx,0x10(%rsp)
    400052a6:	4c 01 f6             	add    %r14,%rsi
    400052a9:	48 89 54 24 08       	mov    %rdx,0x8(%rsp)
    400052ae:	e8 cd 08 00 00       	call   40005b80 <strcmp>
    400052b3:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
    400052b8:	48 8b 4c 24 10       	mov    0x10(%rsp),%rcx
    400052bd:	41 b9 27 00 00 00    	mov    $0x27,%r9d
    400052c3:	85 c0                	test   %eax,%eax
    400052c5:	4c 8b 44 24 18       	mov    0x18(%rsp),%r8
    400052ca:	75 34                	jne    40005300 <__vdsosym+0x220>
    400052cc:	4d 85 c0             	test   %r8,%r8
    400052cf:	0f 84 b0 00 00 00    	je     40005385 <__vdsosym+0x2a5>
    400052d5:	41 0f b7 3c 58       	movzwl (%r8,%rbx,2),%edi
    400052da:	4c 89 fe             	mov    %r15,%rsi
    400052dd:	eb 04                	jmp    400052e3 <__vdsosym+0x203>
    400052df:	90                   	nop
    400052e0:	48 01 c6             	add    %rax,%rsi
    400052e3:	f6 46 02 01          	testb  $0x1,0x2(%rsi)
    400052e7:	75 0c                	jne    400052f5 <__vdsosym+0x215>
    400052e9:	0f b7 46 04          	movzwl 0x4(%rsi),%eax
    400052ed:	31 f8                	xor    %edi,%eax
    400052ef:	66 a9 ff 7f          	test   $0x7fff,%ax
    400052f3:	74 53                	je     40005348 <__vdsosym+0x268>
    400052f5:	8b 46 10             	mov    0x10(%rsi),%eax
    400052f8:	85 c0                	test   %eax,%eax
    400052fa:	75 e4                	jne    400052e0 <__vdsosym+0x200>
    400052fc:	0f 1f 40 00          	nopl   0x0(%rax)
    40005300:	8b 41 04             	mov    0x4(%rcx),%eax
    40005303:	48 83 c3 01          	add    $0x1,%rbx
    40005307:	48 83 c2 18          	add    $0x18,%rdx
    4000530b:	48 39 d8             	cmp    %rbx,%rax
    4000530e:	0f 87 5c ff ff ff    	ja     40005270 <__vdsosym+0x190>
    40005314:	e9 0d fe ff ff       	jmp    40005126 <__vdsosym+0x46>
    40005319:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40005320:	48 89 d1             	mov    %rdx,%rcx
    40005323:	e9 ca fe ff ff       	jmp    400051f2 <__vdsosym+0x112>
    40005328:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4000532f:	00 
    40005330:	49 89 d0             	mov    %rdx,%r8
    40005333:	e9 ba fe ff ff       	jmp    400051f2 <__vdsosym+0x112>
    40005338:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4000533f:	00 
    40005340:	48 89 d6             	mov    %rdx,%rsi
    40005343:	e9 aa fe ff ff       	jmp    400051f2 <__vdsosym+0x112>
    40005348:	8b 46 0c             	mov    0xc(%rsi),%eax
    4000534b:	4c 89 e7             	mov    %r12,%rdi
    4000534e:	48 89 54 24 18       	mov    %rdx,0x18(%rsp)
    40005353:	4c 89 44 24 10       	mov    %r8,0x10(%rsp)
    40005358:	8b 34 06             	mov    (%rsi,%rax,1),%esi
    4000535b:	48 89 4c 24 08       	mov    %rcx,0x8(%rsp)
    40005360:	4c 01 f6             	add    %r14,%rsi
    40005363:	e8 18 08 00 00       	call   40005b80 <strcmp>
    40005368:	48 8b 4c 24 08       	mov    0x8(%rsp),%rcx
    4000536d:	4c 8b 44 24 10       	mov    0x10(%rsp),%r8
    40005372:	41 b9 27 00 00 00    	mov    $0x27,%r9d
    40005378:	85 c0                	test   %eax,%eax
    4000537a:	48 8b 54 24 18       	mov    0x18(%rsp),%rdx
    4000537f:	0f 85 7b ff ff ff    	jne    40005300 <__vdsosym+0x220>
    40005385:	48 8b 42 08          	mov    0x8(%rdx),%rax
    40005389:	4c 01 e8             	add    %r13,%rax
    4000538c:	e9 97 fd ff ff       	jmp    40005128 <__vdsosym+0x48>
    40005391:	b8 08 00 00 00       	mov    $0x8,%eax
    40005396:	e9 ad fd ff ff       	jmp    40005148 <__vdsosym+0x68>
    4000539b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

00000000400053a0 <__lctrans_impl>:
    400053a0:	48 89 f8             	mov    %rdi,%rax
    400053a3:	c3                   	ret    
    400053a4:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    400053ab:	00 00 00 
    400053ae:	66 90                	xchg   %ax,%ax

00000000400053b0 <__lctrans>:
    400053b0:	e9 eb ff ff ff       	jmp    400053a0 <__lctrans_impl>
    400053b5:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    400053bc:	00 00 00 
    400053bf:	90                   	nop

00000000400053c0 <__lctrans_cur>:
    400053c0:	64 48 8b 04 25 00 00 	mov    %fs:0x0,%rax
    400053c7:	00 00 
    400053c9:	48 8b 80 a8 00 00 00 	mov    0xa8(%rax),%rax
    400053d0:	48 8b 70 28          	mov    0x28(%rax),%rsi
    400053d4:	e9 c7 ff ff ff       	jmp    400053a0 <__lctrans_impl>
    400053d9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

00000000400053e0 <__fpclassifyl>:
    400053e0:	48 8b 54 24 08       	mov    0x8(%rsp),%rdx
    400053e5:	48 8b 44 24 10       	mov    0x10(%rsp),%rax
    400053ea:	48 89 d1             	mov    %rdx,%rcx
    400053ed:	89 c6                	mov    %eax,%esi
    400053ef:	25 ff 7f 00 00       	and    $0x7fff,%eax
    400053f4:	48 c1 e9 3f          	shr    $0x3f,%rcx
    400053f8:	66 81 e6 ff 7f       	and    $0x7fff,%si
    400053fd:	09 c8                	or     %ecx,%eax
    400053ff:	74 1f                	je     40005420 <__fpclassifyl+0x40>
    40005401:	8d 04 8d 00 00 00 00 	lea    0x0(,%rcx,4),%eax
    40005408:	66 81 fe ff 7f       	cmp    $0x7fff,%si
    4000540d:	74 01                	je     40005410 <__fpclassifyl+0x30>
    4000540f:	c3                   	ret    
    40005410:	31 c0                	xor    %eax,%eax
    40005412:	48 85 c9             	test   %rcx,%rcx
    40005415:	74 f8                	je     4000540f <__fpclassifyl+0x2f>
    40005417:	31 c0                	xor    %eax,%eax
    40005419:	48 01 d2             	add    %rdx,%rdx
    4000541c:	0f 94 c0             	sete   %al
    4000541f:	c3                   	ret    
    40005420:	48 83 fa 01          	cmp    $0x1,%rdx
    40005424:	b8 02 00 00 00       	mov    $0x2,%eax
    40005429:	83 d8 ff             	sbb    $0xffffffff,%eax
    4000542c:	c3                   	ret    
    4000542d:	0f 1f 00             	nopl   (%rax)

0000000040005430 <__signbitl>:
    40005430:	0f b7 44 24 10       	movzwl 0x10(%rsp),%eax
    40005435:	66 c1 e8 0f          	shr    $0xf,%ax
    40005439:	0f b7 c0             	movzwl %ax,%eax
    4000543c:	c3                   	ret    
    4000543d:	0f 1f 00             	nopl   (%rax)

0000000040005440 <frexpl>:
    40005440:	48 83 ec 28          	sub    $0x28,%rsp
    40005444:	db 6c 24 30          	fldt   0x30(%rsp)
    40005448:	48 8b 44 24 38       	mov    0x38(%rsp),%rax
    4000544d:	89 c2                	mov    %eax,%edx
    4000544f:	d9 c0                	fld    %st(0)
    40005451:	db 7c 24 10          	fstpt  0x10(%rsp)
    40005455:	66 81 e2 ff 7f       	and    $0x7fff,%dx
    4000545a:	74 34                	je     40005490 <frexpl+0x50>
    4000545c:	dd d8                	fstp   %st(0)
    4000545e:	db 6c 24 30          	fldt   0x30(%rsp)
    40005462:	0f b7 ca             	movzwl %dx,%ecx
    40005465:	66 81 fa ff 7f       	cmp    $0x7fff,%dx
    4000546a:	74 1b                	je     40005487 <frexpl+0x47>
    4000546c:	dd d8                	fstp   %st(0)
    4000546e:	66 25 00 80          	and    $0x8000,%ax
    40005472:	81 e9 fe 3f 00 00    	sub    $0x3ffe,%ecx
    40005478:	66 0d fe 3f          	or     $0x3ffe,%ax
    4000547c:	89 0f                	mov    %ecx,(%rdi)
    4000547e:	66 89 44 24 18       	mov    %ax,0x18(%rsp)
    40005483:	db 6c 24 10          	fldt   0x10(%rsp)
    40005487:	48 83 c4 28          	add    $0x28,%rsp
    4000548b:	c3                   	ret    
    4000548c:	0f 1f 40 00          	nopl   0x0(%rax)
    40005490:	d9 ee                	fldz   
    40005492:	d9 c9                	fxch   %st(1)
    40005494:	df e9                	fucomip %st(1),%st
    40005496:	dd d8                	fstp   %st(0)
    40005498:	7a 16                	jp     400054b0 <frexpl+0x70>
    4000549a:	75 14                	jne    400054b0 <frexpl+0x70>
    4000549c:	db 6c 24 30          	fldt   0x30(%rsp)
    400054a0:	c7 07 00 00 00 00    	movl   $0x0,(%rdi)
    400054a6:	48 83 c4 28          	add    $0x28,%rsp
    400054aa:	c3                   	ret    
    400054ab:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    400054b0:	db 6c 24 30          	fldt   0x30(%rsp)
    400054b4:	d8 0d a2 24 00 00    	fmuls  0x24a2(%rip)        # 4000795c <errmsgstr+0x77c>
    400054ba:	48 83 ec 10          	sub    $0x10,%rsp
    400054be:	48 89 7c 24 18       	mov    %rdi,0x18(%rsp)
    400054c3:	db 3c 24             	fstpt  (%rsp)
    400054c6:	e8 75 ff ff ff       	call   40005440 <frexpl>
    400054cb:	48 8b 7c 24 18       	mov    0x18(%rsp),%rdi
    400054d0:	83 2f 78             	subl   $0x78,(%rdi)
    400054d3:	58                   	pop    %rax
    400054d4:	5a                   	pop    %rdx
    400054d5:	48 83 c4 28          	add    $0x28,%rsp
    400054d9:	c3                   	ret    
    400054da:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

00000000400054e0 <wctomb>:
    400054e0:	48 85 ff             	test   %rdi,%rdi
    400054e3:	74 13                	je     400054f8 <wctomb+0x18>
    400054e5:	48 83 ec 08          	sub    $0x8,%rsp
    400054e9:	31 d2                	xor    %edx,%edx
    400054eb:	e8 00 08 00 00       	call   40005cf0 <wcrtomb>
    400054f0:	48 83 c4 08          	add    $0x8,%rsp
    400054f4:	c3                   	ret    
    400054f5:	0f 1f 00             	nopl   (%rax)
    400054f8:	31 c0                	xor    %eax,%eax
    400054fa:	c3                   	ret    
    400054fb:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040005500 <__fstatat>:
    40005500:	55                   	push   %rbp
    40005501:	41 89 f8             	mov    %edi,%r8d
    40005504:	49 89 f1             	mov    %rsi,%r9
    40005507:	53                   	push   %rbx
    40005508:	48 89 d3             	mov    %rdx,%rbx
    4000550b:	48 81 ec b8 00 00 00 	sub    $0xb8,%rsp
    40005512:	48 8d 54 24 20       	lea    0x20(%rsp),%rdx
    40005517:	81 f9 00 10 00 00    	cmp    $0x1000,%ecx
    4000551d:	0f 85 8d 00 00 00    	jne    400055b0 <__fstatat+0xb0>
    40005523:	85 ff                	test   %edi,%edi
    40005525:	0f 88 85 00 00 00    	js     400055b0 <__fstatat+0xb0>
    4000552b:	0f b6 06             	movzbl (%rsi),%eax
    4000552e:	84 c0                	test   %al,%al
    40005530:	0f 85 7a 01 00 00    	jne    400056b0 <__fstatat+0x1b0>
    40005536:	48 63 ef             	movslq %edi,%rbp
    40005539:	b8 05 00 00 00       	mov    $0x5,%eax
    4000553e:	48 89 d6             	mov    %rdx,%rsi
    40005541:	48 89 ef             	mov    %rbp,%rdi
    40005544:	0f 05                	syscall 
    40005546:	41 89 c2             	mov    %eax,%r10d
    40005549:	83 f8 f7             	cmp    $0xfffffff7,%eax
    4000554c:	0f 85 8f 00 00 00    	jne    400055e1 <__fstatat+0xe1>
    40005552:	b8 48 00 00 00       	mov    $0x48,%eax
    40005557:	be 01 00 00 00       	mov    $0x1,%esi
    4000555c:	0f 05                	syscall 
    4000555e:	48 c7 c7 f7 ff ff ff 	mov    $0xfffffffffffffff7,%rdi
    40005565:	48 85 c0             	test   %rax,%rax
    40005568:	0f 88 1b 01 00 00    	js     40005689 <__fstatat+0x189>
    4000556e:	41 ba 00 10 00 00    	mov    $0x1000,%r10d
    40005574:	b8 06 01 00 00       	mov    $0x106,%eax
    40005579:	48 89 ef             	mov    %rbp,%rdi
    4000557c:	4c 89 ce             	mov    %r9,%rsi
    4000557f:	0f 05                	syscall 
    40005581:	41 89 c2             	mov    %eax,%r10d
    40005584:	83 f8 ea             	cmp    $0xffffffea,%eax
    40005587:	75 58                	jne    400055e1 <__fstatat+0xe1>
    40005589:	48 89 e5             	mov    %rsp,%rbp
    4000558c:	44 89 c6             	mov    %r8d,%esi
    4000558f:	48 89 ef             	mov    %rbp,%rdi
    40005592:	e8 b9 06 00 00       	call   40005c50 <__procfdname>
    40005597:	48 8d 54 24 20       	lea    0x20(%rsp),%rdx
    4000559c:	b8 04 00 00 00       	mov    $0x4,%eax
    400055a1:	48 89 ef             	mov    %rbp,%rdi
    400055a4:	48 89 d6             	mov    %rdx,%rsi
    400055a7:	0f 05                	syscall 
    400055a9:	41 89 c2             	mov    %eax,%r10d
    400055ac:	eb 33                	jmp    400055e1 <__fstatat+0xe1>
    400055ae:	66 90                	xchg   %ax,%ax
    400055b0:	41 83 f8 9c          	cmp    $0xffffff9c,%r8d
    400055b4:	0f 85 e6 00 00 00    	jne    400056a0 <__fstatat+0x1a0>
    400055ba:	81 f9 00 01 00 00    	cmp    $0x100,%ecx
    400055c0:	0f 84 12 01 00 00    	je     400056d8 <__fstatat+0x1d8>
    400055c6:	85 c9                	test   %ecx,%ecx
    400055c8:	0f 84 f2 00 00 00    	je     400056c0 <__fstatat+0x1c0>
    400055ce:	49 63 f8             	movslq %r8d,%rdi
    400055d1:	4c 63 d1             	movslq %ecx,%r10
    400055d4:	b8 06 01 00 00       	mov    $0x106,%eax
    400055d9:	4c 89 ce             	mov    %r9,%rsi
    400055dc:	0f 05                	syscall 
    400055de:	41 89 c2             	mov    %eax,%r10d
    400055e1:	49 63 fa             	movslq %r10d,%rdi
    400055e4:	45 85 d2             	test   %r10d,%r10d
    400055e7:	0f 85 9c 00 00 00    	jne    40005689 <__fstatat+0x189>
    400055ed:	31 c0                	xor    %eax,%eax
    400055ef:	b9 12 00 00 00       	mov    $0x12,%ecx
    400055f4:	48 89 df             	mov    %rbx,%rdi
    400055f7:	f3 48 ab             	rep stos %rax,%es:(%rdi)
    400055fa:	48 8b 44 24 20       	mov    0x20(%rsp),%rax
    400055ff:	31 ff                	xor    %edi,%edi
    40005601:	48 89 03             	mov    %rax,(%rbx)
    40005604:	48 8b 44 24 28       	mov    0x28(%rsp),%rax
    40005609:	48 89 43 08          	mov    %rax,0x8(%rbx)
    4000560d:	48 8b 44 24 30       	mov    0x30(%rsp),%rax
    40005612:	48 89 43 10          	mov    %rax,0x10(%rbx)
    40005616:	48 8b 44 24 38       	mov    0x38(%rsp),%rax
    4000561b:	48 89 43 18          	mov    %rax,0x18(%rbx)
    4000561f:	8b 44 24 40          	mov    0x40(%rsp),%eax
    40005623:	89 43 20             	mov    %eax,0x20(%rbx)
    40005626:	48 8b 44 24 48       	mov    0x48(%rsp),%rax
    4000562b:	48 89 43 28          	mov    %rax,0x28(%rbx)
    4000562f:	48 8b 44 24 50       	mov    0x50(%rsp),%rax
    40005634:	48 89 43 30          	mov    %rax,0x30(%rbx)
    40005638:	48 8b 44 24 58       	mov    0x58(%rsp),%rax
    4000563d:	48 89 43 38          	mov    %rax,0x38(%rbx)
    40005641:	48 8b 44 24 60       	mov    0x60(%rsp),%rax
    40005646:	48 89 43 40          	mov    %rax,0x40(%rbx)
    4000564a:	48 8b 44 24 68       	mov    0x68(%rsp),%rax
    4000564f:	48 89 43 48          	mov    %rax,0x48(%rbx)
    40005653:	48 8b 44 24 70       	mov    0x70(%rsp),%rax
    40005658:	48 89 43 50          	mov    %rax,0x50(%rbx)
    4000565c:	48 8b 44 24 78       	mov    0x78(%rsp),%rax
    40005661:	48 89 43 58          	mov    %rax,0x58(%rbx)
    40005665:	48 8b 84 24 80 00 00 	mov    0x80(%rsp),%rax
    4000566c:	00 
    4000566d:	48 89 43 60          	mov    %rax,0x60(%rbx)
    40005671:	48 8b 84 24 88 00 00 	mov    0x88(%rsp),%rax
    40005678:	00 
    40005679:	48 89 43 68          	mov    %rax,0x68(%rbx)
    4000567d:	48 8b 84 24 90 00 00 	mov    0x90(%rsp),%rax
    40005684:	00 
    40005685:	48 89 43 70          	mov    %rax,0x70(%rbx)
    40005689:	e8 b2 c8 ff ff       	call   40001f40 <__syscall_ret>
    4000568e:	48 81 c4 b8 00 00 00 	add    $0xb8,%rsp
    40005695:	5b                   	pop    %rbx
    40005696:	5d                   	pop    %rbp
    40005697:	c3                   	ret    
    40005698:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4000569f:	00 
    400056a0:	41 80 39 2f          	cmpb   $0x2f,(%r9)
    400056a4:	0f 85 24 ff ff ff    	jne    400055ce <__fstatat+0xce>
    400056aa:	e9 0b ff ff ff       	jmp    400055ba <__fstatat+0xba>
    400056af:	90                   	nop
    400056b0:	3c 2f                	cmp    $0x2f,%al
    400056b2:	0f 85 16 ff ff ff    	jne    400055ce <__fstatat+0xce>
    400056b8:	e9 09 ff ff ff       	jmp    400055c6 <__fstatat+0xc6>
    400056bd:	0f 1f 00             	nopl   (%rax)
    400056c0:	b8 04 00 00 00       	mov    $0x4,%eax
    400056c5:	4c 89 cf             	mov    %r9,%rdi
    400056c8:	48 89 d6             	mov    %rdx,%rsi
    400056cb:	0f 05                	syscall 
    400056cd:	41 89 c2             	mov    %eax,%r10d
    400056d0:	e9 0c ff ff ff       	jmp    400055e1 <__fstatat+0xe1>
    400056d5:	0f 1f 00             	nopl   (%rax)
    400056d8:	b8 06 00 00 00       	mov    $0x6,%eax
    400056dd:	4c 89 cf             	mov    %r9,%rdi
    400056e0:	48 89 d6             	mov    %rdx,%rsi
    400056e3:	0f 05                	syscall 
    400056e5:	41 89 c2             	mov    %eax,%r10d
    400056e8:	e9 f4 fe ff ff       	jmp    400055e1 <__fstatat+0xe1>
    400056ed:	0f 1f 00             	nopl   (%rax)

00000000400056f0 <__lockfile>:
    400056f0:	8b 87 8c 00 00 00    	mov    0x8c(%rdi),%eax
    400056f6:	45 31 c0             	xor    %r8d,%r8d
    400056f9:	64 48 8b 0c 25 00 00 	mov    %fs:0x0,%rcx
    40005700:	00 00 
    40005702:	44 8b 49 30          	mov    0x30(%rcx),%r9d
    40005706:	25 ff ff ff bf       	and    $0xbfffffff,%eax
    4000570b:	44 39 c8             	cmp    %r9d,%eax
    4000570e:	0f 84 b2 00 00 00    	je     400057c6 <__lockfile+0xd6>
    40005714:	48 89 fa             	mov    %rdi,%rdx
    40005717:	44 89 c0             	mov    %r8d,%eax
    4000571a:	48 8d bf 8c 00 00 00 	lea    0x8c(%rdi),%rdi
    40005721:	f0 44 0f b1 8a 8c 00 	lock cmpxchg %r9d,0x8c(%rdx)
    40005728:	00 00 
    4000572a:	85 c0                	test   %eax,%eax
    4000572c:	0f 84 8e 00 00 00    	je     400057c0 <__lockfile+0xd0>
    40005732:	41 81 c9 00 00 00 40 	or     $0x40000000,%r9d
    40005739:	44 89 c0             	mov    %r8d,%eax
    4000573c:	f0 44 0f b1 8a 8c 00 	lock cmpxchg %r9d,0x8c(%rdx)
    40005743:	00 00 
    40005745:	89 c1                	mov    %eax,%ecx
    40005747:	85 c0                	test   %eax,%eax
    40005749:	74 75                	je     400057c0 <__lockfile+0xd0>
    4000574b:	55                   	push   %rbp
    4000574c:	45 31 c0             	xor    %r8d,%r8d
    4000574f:	bd 80 00 00 00       	mov    $0x80,%ebp
    40005754:	53                   	push   %rbx
    40005755:	bb ca 00 00 00       	mov    $0xca,%ebx
    4000575a:	eb 1c                	jmp    40005778 <__lockfile+0x88>
    4000575c:	0f 1f 40 00          	nopl   0x0(%rax)
    40005760:	89 c8                	mov    %ecx,%eax
    40005762:	f0 0f b1 17          	lock cmpxchg %edx,(%rdi)
    40005766:	39 c8                	cmp    %ecx,%eax
    40005768:	74 1e                	je     40005788 <__lockfile+0x98>
    4000576a:	44 89 c0             	mov    %r8d,%eax
    4000576d:	f0 44 0f b1 0f       	lock cmpxchg %r9d,(%rdi)
    40005772:	89 c1                	mov    %eax,%ecx
    40005774:	85 c0                	test   %eax,%eax
    40005776:	74 38                	je     400057b0 <__lockfile+0xc0>
    40005778:	89 ca                	mov    %ecx,%edx
    4000577a:	81 ca 00 00 00 40    	or     $0x40000000,%edx
    40005780:	f7 c1 00 00 00 40    	test   $0x40000000,%ecx
    40005786:	74 d8                	je     40005760 <__lockfile+0x70>
    40005788:	48 63 d2             	movslq %edx,%rdx
    4000578b:	45 31 d2             	xor    %r10d,%r10d
    4000578e:	48 89 d8             	mov    %rbx,%rax
    40005791:	48 89 ee             	mov    %rbp,%rsi
    40005794:	0f 05                	syscall 
    40005796:	48 83 f8 da          	cmp    $0xffffffffffffffda,%rax
    4000579a:	75 ce                	jne    4000576a <__lockfile+0x7a>
    4000579c:	45 31 d2             	xor    %r10d,%r10d
    4000579f:	31 f6                	xor    %esi,%esi
    400057a1:	48 89 d8             	mov    %rbx,%rax
    400057a4:	0f 05                	syscall 
    400057a6:	eb c2                	jmp    4000576a <__lockfile+0x7a>
    400057a8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    400057af:	00 
    400057b0:	b8 01 00 00 00       	mov    $0x1,%eax
    400057b5:	5b                   	pop    %rbx
    400057b6:	5d                   	pop    %rbp
    400057b7:	c3                   	ret    
    400057b8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    400057bf:	00 
    400057c0:	41 b8 01 00 00 00    	mov    $0x1,%r8d
    400057c6:	44 89 c0             	mov    %r8d,%eax
    400057c9:	c3                   	ret    
    400057ca:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

00000000400057d0 <__unlockfile>:
    400057d0:	48 89 fa             	mov    %rdi,%rdx
    400057d3:	31 c0                	xor    %eax,%eax
    400057d5:	48 8d bf 8c 00 00 00 	lea    0x8c(%rdi),%rdi
    400057dc:	87 82 8c 00 00 00    	xchg   %eax,0x8c(%rdx)
    400057e2:	a9 00 00 00 40       	test   $0x40000000,%eax
    400057e7:	74 1b                	je     40005804 <__unlockfile+0x34>
    400057e9:	41 b8 ca 00 00 00    	mov    $0xca,%r8d
    400057ef:	ba 01 00 00 00       	mov    $0x1,%edx
    400057f4:	be 81 00 00 00       	mov    $0x81,%esi
    400057f9:	4c 89 c0             	mov    %r8,%rax
    400057fc:	0f 05                	syscall 
    400057fe:	48 83 f8 da          	cmp    $0xffffffffffffffda,%rax
    40005802:	74 04                	je     40005808 <__unlockfile+0x38>
    40005804:	c3                   	ret    
    40005805:	0f 1f 00             	nopl   (%rax)
    40005808:	4c 89 c0             	mov    %r8,%rax
    4000580b:	48 89 d6             	mov    %rdx,%rsi
    4000580e:	0f 05                	syscall 
    40005810:	c3                   	ret    
    40005811:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40005818:	00 00 00 
    4000581b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040005820 <dummy>:
    40005820:	89 f8                	mov    %edi,%eax
    40005822:	c3                   	ret    
    40005823:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    4000582a:	00 00 00 
    4000582d:	0f 1f 00             	nopl   (%rax)

0000000040005830 <__stdio_close>:
    40005830:	48 83 ec 08          	sub    $0x8,%rsp
    40005834:	8b 7f 78             	mov    0x78(%rdi),%edi
    40005837:	e8 94 f3 ff ff       	call   40004bd0 <__aio_close>
    4000583c:	48 63 f8             	movslq %eax,%rdi
    4000583f:	b8 03 00 00 00       	mov    $0x3,%eax
    40005844:	0f 05                	syscall 
    40005846:	48 89 c7             	mov    %rax,%rdi
    40005849:	e8 f2 c6 ff ff       	call   40001f40 <__syscall_ret>
    4000584e:	48 83 c4 08          	add    $0x8,%rsp
    40005852:	c3                   	ret    
    40005853:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    4000585a:	00 00 00 
    4000585d:	0f 1f 00             	nopl   (%rax)

0000000040005860 <__stdio_seek>:
    40005860:	8b 7f 78             	mov    0x78(%rdi),%edi
    40005863:	e9 28 f4 ff ff       	jmp    40004c90 <__lseek>
    40005868:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4000586f:	00 

0000000040005870 <__stdout_write>:
    40005870:	48 83 ec 18          	sub    $0x18,%rsp
    40005874:	49 89 f8             	mov    %rdi,%r8
    40005877:	49 89 f1             	mov    %rsi,%r9
    4000587a:	49 89 d2             	mov    %rdx,%r10
    4000587d:	48 c7 47 48 f0 5e 00 	movq   $0x40005ef0,0x48(%rdi)
    40005884:	40 
    40005885:	f6 07 40             	testb  $0x40,(%rdi)
    40005888:	75 25                	jne    400058af <__stdout_write+0x3f>
    4000588a:	48 63 7f 78          	movslq 0x78(%rdi),%rdi
    4000588e:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
    40005893:	b8 10 00 00 00       	mov    $0x10,%eax
    40005898:	be 13 54 00 00       	mov    $0x5413,%esi
    4000589d:	0f 05                	syscall 
    4000589f:	48 85 c0             	test   %rax,%rax
    400058a2:	74 0b                	je     400058af <__stdout_write+0x3f>
    400058a4:	41 c7 80 90 00 00 00 	movl   $0xffffffff,0x90(%r8)
    400058ab:	ff ff ff ff 
    400058af:	4c 89 d2             	mov    %r10,%rdx
    400058b2:	4c 89 ce             	mov    %r9,%rsi
    400058b5:	4c 89 c7             	mov    %r8,%rdi
    400058b8:	e8 33 06 00 00       	call   40005ef0 <__stdio_write>
    400058bd:	48 83 c4 18          	add    $0x18,%rsp
    400058c1:	c3                   	ret    
    400058c2:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    400058c9:	00 00 00 
    400058cc:	0f 1f 40 00          	nopl   0x0(%rax)

00000000400058d0 <__towrite>:
    400058d0:	8b 97 88 00 00 00    	mov    0x88(%rdi),%edx
    400058d6:	8d 42 ff             	lea    -0x1(%rdx),%eax
    400058d9:	09 d0                	or     %edx,%eax
    400058db:	89 87 88 00 00 00    	mov    %eax,0x88(%rdi)
    400058e1:	8b 07                	mov    (%rdi),%eax
    400058e3:	a8 08                	test   $0x8,%al
    400058e5:	75 29                	jne    40005910 <__towrite+0x40>
    400058e7:	48 8b 47 58          	mov    0x58(%rdi),%rax
    400058eb:	48 c7 47 10 00 00 00 	movq   $0x0,0x10(%rdi)
    400058f2:	00 
    400058f3:	48 c7 47 08 00 00 00 	movq   $0x0,0x8(%rdi)
    400058fa:	00 
    400058fb:	48 89 47 38          	mov    %rax,0x38(%rdi)
    400058ff:	48 89 47 28          	mov    %rax,0x28(%rdi)
    40005903:	48 03 47 60          	add    0x60(%rdi),%rax
    40005907:	48 89 47 20          	mov    %rax,0x20(%rdi)
    4000590b:	31 c0                	xor    %eax,%eax
    4000590d:	c3                   	ret    
    4000590e:	66 90                	xchg   %ax,%ax
    40005910:	83 c8 20             	or     $0x20,%eax
    40005913:	89 07                	mov    %eax,(%rdi)
    40005915:	b8 ff ff ff ff       	mov    $0xffffffff,%eax
    4000591a:	c3                   	ret    
    4000591b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040005920 <__towrite_needs_stdio_exit>:
    40005920:	e9 7b 05 00 00       	jmp    40005ea0 <__stdio_exit>
    40005925:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    4000592c:	00 00 00 
    4000592f:	90                   	nop

0000000040005930 <__fwritex>:
    40005930:	41 56                	push   %r14
    40005932:	41 55                	push   %r13
    40005934:	49 89 f5             	mov    %rsi,%r13
    40005937:	41 54                	push   %r12
    40005939:	49 89 d4             	mov    %rdx,%r12
    4000593c:	55                   	push   %rbp
    4000593d:	48 89 fd             	mov    %rdi,%rbp
    40005940:	53                   	push   %rbx
    40005941:	48 8b 42 20          	mov    0x20(%rdx),%rax
    40005945:	48 85 c0             	test   %rax,%rax
    40005948:	0f 84 9a 00 00 00    	je     400059e8 <__fwritex+0xb8>
    4000594e:	49 8b 7c 24 28       	mov    0x28(%r12),%rdi
    40005953:	48 29 f8             	sub    %rdi,%rax
    40005956:	4c 39 e8             	cmp    %r13,%rax
    40005959:	72 4d                	jb     400059a8 <__fwritex+0x78>
    4000595b:	41 8b 84 24 90 00 00 	mov    0x90(%r12),%eax
    40005962:	00 
    40005963:	4c 89 eb             	mov    %r13,%rbx
    40005966:	85 c0                	test   %eax,%eax
    40005968:	79 34                	jns    4000599e <__fwritex+0x6e>
    4000596a:	4d 89 ee             	mov    %r13,%r14
    4000596d:	4c 89 f2             	mov    %r14,%rdx
    40005970:	48 89 ee             	mov    %rbp,%rsi
    40005973:	e8 35 f0 ff ff       	call   400049ad <memcpy>
    40005978:	4d 01 74 24 28       	add    %r14,0x28(%r12)
    4000597d:	4c 89 e8             	mov    %r13,%rax
    40005980:	5b                   	pop    %rbx
    40005981:	5d                   	pop    %rbp
    40005982:	41 5c                	pop    %r12
    40005984:	41 5d                	pop    %r13
    40005986:	41 5e                	pop    %r14
    40005988:	c3                   	ret    
    40005989:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40005990:	80 7c 1d ff 0a       	cmpb   $0xa,-0x1(%rbp,%rbx,1)
    40005995:	48 8d 43 ff          	lea    -0x1(%rbx),%rax
    40005999:	74 25                	je     400059c0 <__fwritex+0x90>
    4000599b:	48 89 c3             	mov    %rax,%rbx
    4000599e:	48 85 db             	test   %rbx,%rbx
    400059a1:	75 ed                	jne    40005990 <__fwritex+0x60>
    400059a3:	eb c5                	jmp    4000596a <__fwritex+0x3a>
    400059a5:	0f 1f 00             	nopl   (%rax)
    400059a8:	5b                   	pop    %rbx
    400059a9:	49 8b 44 24 48       	mov    0x48(%r12),%rax
    400059ae:	4c 89 ea             	mov    %r13,%rdx
    400059b1:	48 89 ee             	mov    %rbp,%rsi
    400059b4:	4c 89 e7             	mov    %r12,%rdi
    400059b7:	5d                   	pop    %rbp
    400059b8:	41 5c                	pop    %r12
    400059ba:	41 5d                	pop    %r13
    400059bc:	41 5e                	pop    %r14
    400059be:	ff e0                	jmp    *%rax
    400059c0:	48 89 da             	mov    %rbx,%rdx
    400059c3:	48 89 ee             	mov    %rbp,%rsi
    400059c6:	4c 89 e7             	mov    %r12,%rdi
    400059c9:	41 ff 54 24 48       	call   *0x48(%r12)
    400059ce:	48 39 c3             	cmp    %rax,%rbx
    400059d1:	77 ad                	ja     40005980 <__fwritex+0x50>
    400059d3:	4d 89 ee             	mov    %r13,%r14
    400059d6:	49 8b 7c 24 28       	mov    0x28(%r12),%rdi
    400059db:	48 01 dd             	add    %rbx,%rbp
    400059de:	49 29 de             	sub    %rbx,%r14
    400059e1:	eb 8a                	jmp    4000596d <__fwritex+0x3d>
    400059e3:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    400059e8:	48 89 d7             	mov    %rdx,%rdi
    400059eb:	e8 e0 fe ff ff       	call   400058d0 <__towrite>
    400059f0:	85 c0                	test   %eax,%eax
    400059f2:	75 0c                	jne    40005a00 <__fwritex+0xd0>
    400059f4:	49 8b 44 24 20       	mov    0x20(%r12),%rax
    400059f9:	e9 50 ff ff ff       	jmp    4000594e <__fwritex+0x1e>
    400059fe:	66 90                	xchg   %ax,%ax
    40005a00:	31 c0                	xor    %eax,%eax
    40005a02:	e9 79 ff ff ff       	jmp    40005980 <__fwritex+0x50>
    40005a07:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    40005a0e:	00 00 

0000000040005a10 <fwrite>:
    40005a10:	41 57                	push   %r15
    40005a12:	31 c0                	xor    %eax,%eax
    40005a14:	41 56                	push   %r14
    40005a16:	49 89 f6             	mov    %rsi,%r14
    40005a19:	41 55                	push   %r13
    40005a1b:	4c 0f af f2          	imul   %rdx,%r14
    40005a1f:	49 89 fd             	mov    %rdi,%r13
    40005a22:	41 54                	push   %r12
    40005a24:	49 89 cc             	mov    %rcx,%r12
    40005a27:	55                   	push   %rbp
    40005a28:	48 89 f5             	mov    %rsi,%rbp
    40005a2b:	53                   	push   %rbx
    40005a2c:	48 89 d3             	mov    %rdx,%rbx
    40005a2f:	48 83 ec 08          	sub    $0x8,%rsp
    40005a33:	48 85 f6             	test   %rsi,%rsi
    40005a36:	48 0f 44 d8          	cmove  %rax,%rbx
    40005a3a:	8b 81 8c 00 00 00    	mov    0x8c(%rcx),%eax
    40005a40:	85 c0                	test   %eax,%eax
    40005a42:	79 34                	jns    40005a78 <fwrite+0x68>
    40005a44:	48 89 ca             	mov    %rcx,%rdx
    40005a47:	4c 89 f6             	mov    %r14,%rsi
    40005a4a:	e8 e1 fe ff ff       	call   40005930 <__fwritex>
    40005a4f:	49 89 c5             	mov    %rax,%r13
    40005a52:	48 89 d8             	mov    %rbx,%rax
    40005a55:	4d 39 ee             	cmp    %r13,%r14
    40005a58:	74 08                	je     40005a62 <fwrite+0x52>
    40005a5a:	4c 89 e8             	mov    %r13,%rax
    40005a5d:	31 d2                	xor    %edx,%edx
    40005a5f:	48 f7 f5             	div    %rbp
    40005a62:	48 83 c4 08          	add    $0x8,%rsp
    40005a66:	5b                   	pop    %rbx
    40005a67:	5d                   	pop    %rbp
    40005a68:	41 5c                	pop    %r12
    40005a6a:	41 5d                	pop    %r13
    40005a6c:	41 5e                	pop    %r14
    40005a6e:	41 5f                	pop    %r15
    40005a70:	c3                   	ret    
    40005a71:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40005a78:	48 89 cf             	mov    %rcx,%rdi
    40005a7b:	e8 70 fc ff ff       	call   400056f0 <__lockfile>
    40005a80:	4c 89 ef             	mov    %r13,%rdi
    40005a83:	4c 89 e2             	mov    %r12,%rdx
    40005a86:	4c 89 f6             	mov    %r14,%rsi
    40005a89:	41 89 c7             	mov    %eax,%r15d
    40005a8c:	e8 9f fe ff ff       	call   40005930 <__fwritex>
    40005a91:	49 89 c5             	mov    %rax,%r13
    40005a94:	45 85 ff             	test   %r15d,%r15d
    40005a97:	74 b9                	je     40005a52 <fwrite+0x42>
    40005a99:	4c 89 e7             	mov    %r12,%rdi
    40005a9c:	e8 2f fd ff ff       	call   400057d0 <__unlockfile>
    40005aa1:	eb af                	jmp    40005a52 <fwrite+0x42>
    40005aa3:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40005aaa:	00 00 00 
    40005aad:	0f 1f 00             	nopl   (%rax)

0000000040005ab0 <memchr>:
    40005ab0:	48 89 f8             	mov    %rdi,%rax
    40005ab3:	44 0f b6 d6          	movzbl %sil,%r10d
    40005ab7:	a8 07                	test   $0x7,%al
    40005ab9:	75 19                	jne    40005ad4 <memchr+0x24>
    40005abb:	eb 23                	jmp    40005ae0 <memchr+0x30>
    40005abd:	0f 1f 00             	nopl   (%rax)
    40005ac0:	0f b6 08             	movzbl (%rax),%ecx
    40005ac3:	44 39 d1             	cmp    %r10d,%ecx
    40005ac6:	74 7b                	je     40005b43 <memchr+0x93>
    40005ac8:	48 83 c0 01          	add    $0x1,%rax
    40005acc:	48 83 ea 01          	sub    $0x1,%rdx
    40005ad0:	a8 07                	test   $0x7,%al
    40005ad2:	74 0c                	je     40005ae0 <memchr+0x30>
    40005ad4:	48 85 d2             	test   %rdx,%rdx
    40005ad7:	75 e7                	jne    40005ac0 <memchr+0x10>
    40005ad9:	45 31 c0             	xor    %r8d,%r8d
    40005adc:	4c 89 c0             	mov    %r8,%rax
    40005adf:	c3                   	ret    
    40005ae0:	45 31 c0             	xor    %r8d,%r8d
    40005ae3:	48 85 d2             	test   %rdx,%rdx
    40005ae6:	74 f4                	je     40005adc <memchr+0x2c>
    40005ae8:	0f b6 08             	movzbl (%rax),%ecx
    40005aeb:	44 39 d1             	cmp    %r10d,%ecx
    40005aee:	74 53                	je     40005b43 <memchr+0x93>
    40005af0:	48 b9 01 01 01 01 01 	movabs $0x101010101010101,%rcx
    40005af7:	01 01 01 
    40005afa:	49 63 fa             	movslq %r10d,%rdi
    40005afd:	49 b9 ff fe fe fe fe 	movabs $0xfefefefefefefeff,%r9
    40005b04:	fe fe fe 
    40005b07:	49 b8 80 80 80 80 80 	movabs $0x8080808080808080,%r8
    40005b0e:	80 80 80 
    40005b11:	48 0f af f9          	imul   %rcx,%rdi
    40005b15:	48 83 fa 07          	cmp    $0x7,%rdx
    40005b19:	77 13                	ja     40005b2e <memchr+0x7e>
    40005b1b:	eb 26                	jmp    40005b43 <memchr+0x93>
    40005b1d:	0f 1f 00             	nopl   (%rax)
    40005b20:	48 83 ea 08          	sub    $0x8,%rdx
    40005b24:	48 83 c0 08          	add    $0x8,%rax
    40005b28:	48 83 fa 07          	cmp    $0x7,%rdx
    40005b2c:	76 39                	jbe    40005b67 <memchr+0xb7>
    40005b2e:	48 8b 08             	mov    (%rax),%rcx
    40005b31:	48 31 f9             	xor    %rdi,%rcx
    40005b34:	4a 8d 34 09          	lea    (%rcx,%r9,1),%rsi
    40005b38:	48 f7 d1             	not    %rcx
    40005b3b:	48 21 f1             	and    %rsi,%rcx
    40005b3e:	4c 85 c1             	test   %r8,%rcx
    40005b41:	74 dd                	je     40005b20 <memchr+0x70>
    40005b43:	49 89 c0             	mov    %rax,%r8
    40005b46:	48 01 c2             	add    %rax,%rdx
    40005b49:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40005b50:	41 0f b6 00          	movzbl (%r8),%eax
    40005b54:	44 39 d0             	cmp    %r10d,%eax
    40005b57:	74 83                	je     40005adc <memchr+0x2c>
    40005b59:	49 83 c0 01          	add    $0x1,%r8
    40005b5d:	4c 39 c2             	cmp    %r8,%rdx
    40005b60:	75 ee                	jne    40005b50 <memchr+0xa0>
    40005b62:	e9 72 ff ff ff       	jmp    40005ad9 <memchr+0x29>
    40005b67:	45 31 c0             	xor    %r8d,%r8d
    40005b6a:	48 85 d2             	test   %rdx,%rdx
    40005b6d:	0f 84 69 ff ff ff    	je     40005adc <memchr+0x2c>
    40005b73:	eb ce                	jmp    40005b43 <memchr+0x93>
    40005b75:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40005b7c:	00 00 00 
    40005b7f:	90                   	nop

0000000040005b80 <strcmp>:
    40005b80:	0f b6 17             	movzbl (%rdi),%edx
    40005b83:	0f b6 0e             	movzbl (%rsi),%ecx
    40005b86:	b8 01 00 00 00       	mov    $0x1,%eax
    40005b8b:	38 d1                	cmp    %dl,%cl
    40005b8d:	74 1a                	je     40005ba9 <strcmp+0x29>
    40005b8f:	eb 27                	jmp    40005bb8 <strcmp+0x38>
    40005b91:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40005b98:	0f b6 14 07          	movzbl (%rdi,%rax,1),%edx
    40005b9c:	48 83 c0 01          	add    $0x1,%rax
    40005ba0:	0f b6 4c 06 ff       	movzbl -0x1(%rsi,%rax,1),%ecx
    40005ba5:	38 ca                	cmp    %cl,%dl
    40005ba7:	75 0f                	jne    40005bb8 <strcmp+0x38>
    40005ba9:	84 d2                	test   %dl,%dl
    40005bab:	75 eb                	jne    40005b98 <strcmp+0x18>
    40005bad:	31 c0                	xor    %eax,%eax
    40005baf:	29 c8                	sub    %ecx,%eax
    40005bb1:	c3                   	ret    
    40005bb2:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40005bb8:	0f b6 c2             	movzbl %dl,%eax
    40005bbb:	29 c8                	sub    %ecx,%eax
    40005bbd:	c3                   	ret    

0000000040005bbe <__set_thread_area>:
    40005bbe:	48 89 fe             	mov    %rdi,%rsi
    40005bc1:	bf 02 10 00 00       	mov    $0x1002,%edi
    40005bc6:	b8 9e 00 00 00       	mov    $0x9e,%eax
    40005bcb:	0f 05                	syscall 
    40005bcd:	c3                   	ret    
    40005bce:	66 90                	xchg   %ax,%ax

0000000040005bd0 <__clock_nanosleep>:
    40005bd0:	83 ff 03             	cmp    $0x3,%edi
    40005bd3:	74 6b                	je     40005c40 <__clock_nanosleep+0x70>
    40005bd5:	89 f8                	mov    %edi,%eax
    40005bd7:	48 83 ec 08          	sub    $0x8,%rsp
    40005bdb:	49 89 d2             	mov    %rdx,%r10
    40005bde:	49 89 cb             	mov    %rcx,%r11
    40005be1:	09 f0                	or     %esi,%eax
    40005be3:	74 2b                	je     40005c10 <__clock_nanosleep+0x40>
    40005be5:	48 83 ec 08          	sub    $0x8,%rsp
    40005be9:	48 63 d6             	movslq %esi,%rdx
    40005bec:	49 89 c8             	mov    %rcx,%r8
    40005bef:	48 63 f7             	movslq %edi,%rsi
    40005bf2:	6a 00                	push   $0x0
    40005bf4:	4c 89 d1             	mov    %r10,%rcx
    40005bf7:	45 31 c9             	xor    %r9d,%r9d
    40005bfa:	bf e6 00 00 00       	mov    $0xe6,%edi
    40005bff:	e8 b6 b9 ff ff       	call   400015ba <__syscall_cp>
    40005c04:	5a                   	pop    %rdx
    40005c05:	59                   	pop    %rcx
    40005c06:	f7 d8                	neg    %eax
    40005c08:	48 83 c4 08          	add    $0x8,%rsp
    40005c0c:	c3                   	ret    
    40005c0d:	0f 1f 00             	nopl   (%rax)
    40005c10:	48 83 ec 08          	sub    $0x8,%rsp
    40005c14:	4c 89 d6             	mov    %r10,%rsi
    40005c17:	bf 23 00 00 00       	mov    $0x23,%edi
    40005c1c:	45 31 c9             	xor    %r9d,%r9d
    40005c1f:	6a 00                	push   $0x0
    40005c21:	45 31 c0             	xor    %r8d,%r8d
    40005c24:	31 c9                	xor    %ecx,%ecx
    40005c26:	4c 89 da             	mov    %r11,%rdx
    40005c29:	e8 8c b9 ff ff       	call   400015ba <__syscall_cp>
    40005c2e:	5e                   	pop    %rsi
    40005c2f:	5f                   	pop    %rdi
    40005c30:	f7 d8                	neg    %eax
    40005c32:	48 83 c4 08          	add    $0x8,%rsp
    40005c36:	c3                   	ret    
    40005c37:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    40005c3e:	00 00 
    40005c40:	b8 16 00 00 00       	mov    $0x16,%eax
    40005c45:	c3                   	ret    
    40005c46:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40005c4d:	00 00 00 

0000000040005c50 <__procfdname>:
    40005c50:	c6 07 2f             	movb   $0x2f,(%rdi)
    40005c53:	b8 01 00 00 00       	mov    $0x1,%eax
    40005c58:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    40005c5f:	00 
    40005c60:	0f b6 88 60 79 00 40 	movzbl 0x40007960(%rax),%ecx
    40005c67:	89 c2                	mov    %eax,%edx
    40005c69:	4c 8d 04 07          	lea    (%rdi,%rax,1),%r8
    40005c6d:	88 0c 07             	mov    %cl,(%rdi,%rax,1)
    40005c70:	48 83 c0 01          	add    $0x1,%rax
    40005c74:	84 c9                	test   %cl,%cl
    40005c76:	75 e8                	jne    40005c60 <__procfdname+0x10>
    40005c78:	89 f1                	mov    %esi,%ecx
    40005c7a:	41 b9 cd cc cc cc    	mov    $0xcccccccd,%r9d
    40005c80:	85 f6                	test   %esi,%esi
    40005c82:	74 5c                	je     40005ce0 <__procfdname+0x90>
    40005c84:	0f 1f 40 00          	nopl   0x0(%rax)
    40005c88:	89 c9                	mov    %ecx,%ecx
    40005c8a:	83 c2 01             	add    $0x1,%edx
    40005c8d:	48 89 c8             	mov    %rcx,%rax
    40005c90:	49 0f af c9          	imul   %r9,%rcx
    40005c94:	48 c1 e9 23          	shr    $0x23,%rcx
    40005c98:	83 f8 09             	cmp    $0x9,%eax
    40005c9b:	77 eb                	ja     40005c88 <__procfdname+0x38>
    40005c9d:	89 d0                	mov    %edx,%eax
    40005c9f:	41 ba cd cc cc cc    	mov    $0xcccccccd,%r10d
    40005ca5:	c6 04 07 00          	movb   $0x0,(%rdi,%rax,1)
    40005ca9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40005cb0:	89 f0                	mov    %esi,%eax
    40005cb2:	89 f1                	mov    %esi,%ecx
    40005cb4:	44 8d 4a ff          	lea    -0x1(%rdx),%r9d
    40005cb8:	49 0f af c2          	imul   %r10,%rax
    40005cbc:	4c 89 ca             	mov    %r9,%rdx
    40005cbf:	48 c1 e8 23          	shr    $0x23,%rax
    40005cc3:	44 8d 04 80          	lea    (%rax,%rax,4),%r8d
    40005cc7:	45 01 c0             	add    %r8d,%r8d
    40005cca:	44 29 c1             	sub    %r8d,%ecx
    40005ccd:	83 c1 30             	add    $0x30,%ecx
    40005cd0:	42 88 0c 0f          	mov    %cl,(%rdi,%r9,1)
    40005cd4:	89 f1                	mov    %esi,%ecx
    40005cd6:	89 c6                	mov    %eax,%esi
    40005cd8:	83 f9 09             	cmp    $0x9,%ecx
    40005cdb:	77 d3                	ja     40005cb0 <__procfdname+0x60>
    40005cdd:	c3                   	ret    
    40005cde:	66 90                	xchg   %ax,%ax
    40005ce0:	41 c6 00 30          	movb   $0x30,(%r8)
    40005ce4:	c6 04 07 00          	movb   $0x0,(%rdi,%rax,1)
    40005ce8:	c3                   	ret    
    40005ce9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000040005cf0 <wcrtomb>:
    40005cf0:	48 85 ff             	test   %rdi,%rdi
    40005cf3:	74 7e                	je     40005d73 <wcrtomb+0x83>
    40005cf5:	48 89 fa             	mov    %rdi,%rdx
    40005cf8:	89 f0                	mov    %esi,%eax
    40005cfa:	83 fe 7f             	cmp    $0x7f,%esi
    40005cfd:	76 71                	jbe    40005d70 <wcrtomb+0x80>
    40005cff:	48 83 ec 08          	sub    $0x8,%rsp
    40005d03:	64 48 8b 0c 25 00 00 	mov    %fs:0x0,%rcx
    40005d0a:	00 00 
    40005d0c:	48 8b 89 a8 00 00 00 	mov    0xa8(%rcx),%rcx
    40005d13:	48 83 39 00          	cmpq   $0x0,(%rcx)
    40005d17:	74 67                	je     40005d80 <wcrtomb+0x90>
    40005d19:	81 fe ff 07 00 00    	cmp    $0x7ff,%esi
    40005d1f:	0f 86 bb 00 00 00    	jbe    40005de0 <wcrtomb+0xf0>
    40005d25:	8d 8e 00 20 ff ff    	lea    -0xe000(%rsi),%ecx
    40005d2b:	81 f9 ff 1f 00 00    	cmp    $0x1fff,%ecx
    40005d31:	0f 86 c9 00 00 00    	jbe    40005e00 <wcrtomb+0x110>
    40005d37:	81 fe ff d7 00 00    	cmp    $0xd7ff,%esi
    40005d3d:	0f 86 bd 00 00 00    	jbe    40005e00 <wcrtomb+0x110>
    40005d43:	8d 8e 00 00 ff ff    	lea    -0x10000(%rsi),%ecx
    40005d49:	81 f9 ff ff 0f 00    	cmp    $0xfffff,%ecx
    40005d4f:	76 4f                	jbe    40005da0 <wcrtomb+0xb0>
    40005d51:	e8 ea f2 ff ff       	call   40005040 <__errno_location>
    40005d56:	c7 00 54 00 00 00    	movl   $0x54,(%rax)
    40005d5c:	48 c7 c0 ff ff ff ff 	mov    $0xffffffffffffffff,%rax
    40005d63:	48 83 c4 08          	add    $0x8,%rsp
    40005d67:	c3                   	ret    
    40005d68:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    40005d6f:	00 
    40005d70:	40 88 32             	mov    %sil,(%rdx)
    40005d73:	b8 01 00 00 00       	mov    $0x1,%eax
    40005d78:	c3                   	ret    
    40005d79:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40005d80:	8d 8e 80 20 ff ff    	lea    -0xdf80(%rsi),%ecx
    40005d86:	83 f9 7f             	cmp    $0x7f,%ecx
    40005d89:	77 c6                	ja     40005d51 <wcrtomb+0x61>
    40005d8b:	88 02                	mov    %al,(%rdx)
    40005d8d:	b8 01 00 00 00       	mov    $0x1,%eax
    40005d92:	48 83 c4 08          	add    $0x8,%rsp
    40005d96:	c3                   	ret    
    40005d97:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    40005d9e:	00 00 
    40005da0:	89 f1                	mov    %esi,%ecx
    40005da2:	83 e0 3f             	and    $0x3f,%eax
    40005da5:	c1 f9 12             	sar    $0x12,%ecx
    40005da8:	83 c8 80             	or     $0xffffff80,%eax
    40005dab:	83 c9 f0             	or     $0xfffffff0,%ecx
    40005dae:	88 47 03             	mov    %al,0x3(%rdi)
    40005db1:	b8 04 00 00 00       	mov    $0x4,%eax
    40005db6:	88 0f                	mov    %cl,(%rdi)
    40005db8:	89 f1                	mov    %esi,%ecx
    40005dba:	c1 f9 0c             	sar    $0xc,%ecx
    40005dbd:	83 e1 3f             	and    $0x3f,%ecx
    40005dc0:	83 c9 80             	or     $0xffffff80,%ecx
    40005dc3:	88 4f 01             	mov    %cl,0x1(%rdi)
    40005dc6:	89 f1                	mov    %esi,%ecx
    40005dc8:	c1 f9 06             	sar    $0x6,%ecx
    40005dcb:	83 e1 3f             	and    $0x3f,%ecx
    40005dce:	83 c9 80             	or     $0xffffff80,%ecx
    40005dd1:	88 4f 02             	mov    %cl,0x2(%rdi)
    40005dd4:	eb 8d                	jmp    40005d63 <wcrtomb+0x73>
    40005dd6:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40005ddd:	00 00 00 
    40005de0:	89 f1                	mov    %esi,%ecx
    40005de2:	83 e0 3f             	and    $0x3f,%eax
    40005de5:	c1 f9 06             	sar    $0x6,%ecx
    40005de8:	83 c8 80             	or     $0xffffff80,%eax
    40005deb:	83 c9 c0             	or     $0xffffffc0,%ecx
    40005dee:	88 47 01             	mov    %al,0x1(%rdi)
    40005df1:	b8 02 00 00 00       	mov    $0x2,%eax
    40005df6:	88 0f                	mov    %cl,(%rdi)
    40005df8:	e9 66 ff ff ff       	jmp    40005d63 <wcrtomb+0x73>
    40005dfd:	0f 1f 00             	nopl   (%rax)
    40005e00:	89 c1                	mov    %eax,%ecx
    40005e02:	c1 f9 0c             	sar    $0xc,%ecx
    40005e05:	83 c9 e0             	or     $0xffffffe0,%ecx
    40005e08:	88 0a                	mov    %cl,(%rdx)
    40005e0a:	89 c1                	mov    %eax,%ecx
    40005e0c:	83 e0 3f             	and    $0x3f,%eax
    40005e0f:	c1 f9 06             	sar    $0x6,%ecx
    40005e12:	83 c8 80             	or     $0xffffff80,%eax
    40005e15:	83 e1 3f             	and    $0x3f,%ecx
    40005e18:	88 42 02             	mov    %al,0x2(%rdx)
    40005e1b:	b8 03 00 00 00       	mov    $0x3,%eax
    40005e20:	83 c9 80             	or     $0xffffff80,%ecx
    40005e23:	88 4a 01             	mov    %cl,0x1(%rdx)
    40005e26:	e9 38 ff ff ff       	jmp    40005d63 <wcrtomb+0x73>
    40005e2b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)

0000000040005e30 <close_file>:
    40005e30:	48 85 ff             	test   %rdi,%rdi
    40005e33:	74 63                	je     40005e98 <close_file+0x68>
    40005e35:	55                   	push   %rbp
    40005e36:	8b 87 8c 00 00 00    	mov    0x8c(%rdi),%eax
    40005e3c:	48 89 fd             	mov    %rdi,%rbp
    40005e3f:	85 c0                	test   %eax,%eax
    40005e41:	79 3d                	jns    40005e80 <close_file+0x50>
    40005e43:	48 8b 45 38          	mov    0x38(%rbp),%rax
    40005e47:	48 39 45 28          	cmp    %rax,0x28(%rbp)
    40005e4b:	74 0a                	je     40005e57 <close_file+0x27>
    40005e4d:	31 d2                	xor    %edx,%edx
    40005e4f:	31 f6                	xor    %esi,%esi
    40005e51:	48 89 ef             	mov    %rbp,%rdi
    40005e54:	ff 55 48             	call   *0x48(%rbp)
    40005e57:	48 8b 75 08          	mov    0x8(%rbp),%rsi
    40005e5b:	48 8b 45 10          	mov    0x10(%rbp),%rax
    40005e5f:	48 39 c6             	cmp    %rax,%rsi
    40005e62:	74 2c                	je     40005e90 <close_file+0x60>
    40005e64:	48 29 c6             	sub    %rax,%rsi
    40005e67:	48 89 ef             	mov    %rbp,%rdi
    40005e6a:	48 8b 45 50          	mov    0x50(%rbp),%rax
    40005e6e:	ba 01 00 00 00       	mov    $0x1,%edx
    40005e73:	5d                   	pop    %rbp
    40005e74:	ff e0                	jmp    *%rax
    40005e76:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40005e7d:	00 00 00 
    40005e80:	e8 6b f8 ff ff       	call   400056f0 <__lockfile>
    40005e85:	eb bc                	jmp    40005e43 <close_file+0x13>
    40005e87:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    40005e8e:	00 00 
    40005e90:	5d                   	pop    %rbp
    40005e91:	c3                   	ret    
    40005e92:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    40005e98:	c3                   	ret    
    40005e99:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000040005ea0 <__stdio_exit>:
    40005ea0:	53                   	push   %rbx
    40005ea1:	e8 5a 01 00 00       	call   40006000 <__ofl_lock>
    40005ea6:	48 8b 18             	mov    (%rax),%rbx
    40005ea9:	48 85 db             	test   %rbx,%rbx
    40005eac:	74 13                	je     40005ec1 <__stdio_exit+0x21>
    40005eae:	66 90                	xchg   %ax,%ax
    40005eb0:	48 89 df             	mov    %rbx,%rdi
    40005eb3:	e8 78 ff ff ff       	call   40005e30 <close_file>
    40005eb8:	48 8b 5b 70          	mov    0x70(%rbx),%rbx
    40005ebc:	48 85 db             	test   %rbx,%rbx
    40005ebf:	75 ef                	jne    40005eb0 <__stdio_exit+0x10>
    40005ec1:	48 8b 3d 10 25 00 00 	mov    0x2510(%rip),%rdi        # 400083d8 <__stderr_used>
    40005ec8:	e8 63 ff ff ff       	call   40005e30 <close_file>
    40005ecd:	48 8b 3d 8c 1d 00 00 	mov    0x1d8c(%rip),%rdi        # 40007c60 <__stdout_used>
    40005ed4:	e8 57 ff ff ff       	call   40005e30 <close_file>
    40005ed9:	48 8b 3d f8 24 00 00 	mov    0x24f8(%rip),%rdi        # 400083d8 <__stderr_used>
    40005ee0:	5b                   	pop    %rbx
    40005ee1:	e9 4a ff ff ff       	jmp    40005e30 <close_file>
    40005ee6:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40005eed:	00 00 00 

0000000040005ef0 <__stdio_write>:
    40005ef0:	41 57                	push   %r15
    40005ef2:	49 89 ff             	mov    %rdi,%r15
    40005ef5:	41 56                	push   %r14
    40005ef7:	41 55                	push   %r13
    40005ef9:	41 54                	push   %r12
    40005efb:	41 bc 14 00 00 00    	mov    $0x14,%r12d
    40005f01:	55                   	push   %rbp
    40005f02:	bd 02 00 00 00       	mov    $0x2,%ebp
    40005f07:	53                   	push   %rbx
    40005f08:	bb 02 00 00 00       	mov    $0x2,%ebx
    40005f0d:	48 83 ec 38          	sub    $0x38,%rsp
    40005f11:	48 8b 47 38          	mov    0x38(%rdi),%rax
    40005f15:	4c 8b 77 28          	mov    0x28(%rdi),%r14
    40005f19:	48 89 54 24 08       	mov    %rdx,0x8(%rsp)
    40005f1e:	4c 8d 6c 24 10       	lea    0x10(%rsp),%r13
    40005f23:	49 29 c6             	sub    %rax,%r14
    40005f26:	48 89 44 24 10       	mov    %rax,0x10(%rsp)
    40005f2b:	4c 89 74 24 18       	mov    %r14,0x18(%rsp)
    40005f30:	49 01 d6             	add    %rdx,%r14
    40005f33:	48 89 74 24 20       	mov    %rsi,0x20(%rsp)
    40005f38:	48 89 54 24 28       	mov    %rdx,0x28(%rsp)
    40005f3d:	eb 2e                	jmp    40005f6d <__stdio_write+0x7d>
    40005f3f:	90                   	nop
    40005f40:	48 85 c0             	test   %rax,%rax
    40005f43:	78 73                	js     40005fb8 <__stdio_write+0xc8>
    40005f45:	49 8b 55 08          	mov    0x8(%r13),%rdx
    40005f49:	49 29 c6             	sub    %rax,%r14
    40005f4c:	48 39 d0             	cmp    %rdx,%rax
    40005f4f:	76 11                	jbe    40005f62 <__stdio_write+0x72>
    40005f51:	83 ed 01             	sub    $0x1,%ebp
    40005f54:	48 29 d0             	sub    %rdx,%rax
    40005f57:	49 83 c5 10          	add    $0x10,%r13
    40005f5b:	49 8b 55 08          	mov    0x8(%r13),%rdx
    40005f5f:	48 63 dd             	movslq %ebp,%rbx
    40005f62:	48 29 c2             	sub    %rax,%rdx
    40005f65:	49 01 45 00          	add    %rax,0x0(%r13)
    40005f69:	49 89 55 08          	mov    %rdx,0x8(%r13)
    40005f6d:	49 63 7f 78          	movslq 0x78(%r15),%rdi
    40005f71:	4c 89 e0             	mov    %r12,%rax
    40005f74:	4c 89 ee             	mov    %r13,%rsi
    40005f77:	48 89 da             	mov    %rbx,%rdx
    40005f7a:	0f 05                	syscall 
    40005f7c:	48 89 c7             	mov    %rax,%rdi
    40005f7f:	e8 bc bf ff ff       	call   40001f40 <__syscall_ret>
    40005f84:	49 39 c6             	cmp    %rax,%r14
    40005f87:	75 b7                	jne    40005f40 <__stdio_write+0x50>
    40005f89:	49 8b 47 58          	mov    0x58(%r15),%rax
    40005f8d:	49 8b 57 60          	mov    0x60(%r15),%rdx
    40005f91:	48 01 c2             	add    %rax,%rdx
    40005f94:	49 89 47 38          	mov    %rax,0x38(%r15)
    40005f98:	49 89 57 20          	mov    %rdx,0x20(%r15)
    40005f9c:	49 89 47 28          	mov    %rax,0x28(%r15)
    40005fa0:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
    40005fa5:	48 83 c4 38          	add    $0x38,%rsp
    40005fa9:	5b                   	pop    %rbx
    40005faa:	5d                   	pop    %rbp
    40005fab:	41 5c                	pop    %r12
    40005fad:	41 5d                	pop    %r13
    40005faf:	41 5e                	pop    %r14
    40005fb1:	41 5f                	pop    %r15
    40005fb3:	c3                   	ret    
    40005fb4:	0f 1f 40 00          	nopl   0x0(%rax)
    40005fb8:	41 83 0f 20          	orl    $0x20,(%r15)
    40005fbc:	31 c0                	xor    %eax,%eax
    40005fbe:	49 c7 47 20 00 00 00 	movq   $0x0,0x20(%r15)
    40005fc5:	00 
    40005fc6:	49 c7 47 38 00 00 00 	movq   $0x0,0x38(%r15)
    40005fcd:	00 
    40005fce:	49 c7 47 28 00 00 00 	movq   $0x0,0x28(%r15)
    40005fd5:	00 
    40005fd6:	83 fd 02             	cmp    $0x2,%ebp
    40005fd9:	74 ca                	je     40005fa5 <__stdio_write+0xb5>
    40005fdb:	48 8b 44 24 08       	mov    0x8(%rsp),%rax
    40005fe0:	49 2b 45 08          	sub    0x8(%r13),%rax
    40005fe4:	48 83 c4 38          	add    $0x38,%rsp
    40005fe8:	5b                   	pop    %rbx
    40005fe9:	5d                   	pop    %rbp
    40005fea:	41 5c                	pop    %r12
    40005fec:	41 5d                	pop    %r13
    40005fee:	41 5e                	pop    %r14
    40005ff0:	41 5f                	pop    %r15
    40005ff2:	c3                   	ret    
    40005ff3:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40005ffa:	00 00 00 
    40005ffd:	0f 1f 00             	nopl   (%rax)

0000000040006000 <__ofl_lock>:
    40006000:	48 83 ec 08          	sub    $0x8,%rsp
    40006004:	bf e0 83 00 40       	mov    $0x400083e0,%edi
    40006009:	e8 22 00 00 00       	call   40006030 <__lock>
    4000600e:	b8 e8 83 00 40       	mov    $0x400083e8,%eax
    40006013:	48 83 c4 08          	add    $0x8,%rsp
    40006017:	c3                   	ret    
    40006018:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4000601f:	00 

0000000040006020 <__ofl_unlock>:
    40006020:	bf e0 83 00 40       	mov    $0x400083e0,%edi
    40006025:	e9 f6 00 00 00       	jmp    40006120 <__unlock>
    4000602a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)

0000000040006030 <__lock>:
    40006030:	0f be 0d 6c 1d 00 00 	movsbl 0x1d6c(%rip),%ecx        # 40007da3 <__libc+0x3>
    40006037:	85 c9                	test   %ecx,%ecx
    40006039:	74 55                	je     40006090 <__lock+0x60>
    4000603b:	31 c0                	xor    %eax,%eax
    4000603d:	ba 01 00 00 80       	mov    $0x80000001,%edx
    40006042:	f0 0f b1 17          	lock cmpxchg %edx,(%rdi)
    40006046:	89 c2                	mov    %eax,%edx
    40006048:	85 c9                	test   %ecx,%ecx
    4000604a:	78 4c                	js     40006098 <__lock+0x68>
    4000604c:	85 c0                	test   %eax,%eax
    4000604e:	74 40                	je     40006090 <__lock+0x60>
    40006050:	41 b8 0a 00 00 00    	mov    $0xa,%r8d
    40006056:	eb 22                	jmp    4000607a <__lock+0x4a>
    40006058:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4000605f:	00 
    40006060:	8d b2 01 00 00 80    	lea    -0x7fffffff(%rdx),%esi
    40006066:	89 d1                	mov    %edx,%ecx
    40006068:	89 c8                	mov    %ecx,%eax
    4000606a:	f0 0f b1 37          	lock cmpxchg %esi,(%rdi)
    4000606e:	89 c2                	mov    %eax,%edx
    40006070:	39 c1                	cmp    %eax,%ecx
    40006072:	74 1c                	je     40006090 <__lock+0x60>
    40006074:	41 83 e8 01          	sub    $0x1,%r8d
    40006078:	74 2e                	je     400060a8 <__lock+0x78>
    4000607a:	85 d2                	test   %edx,%edx
    4000607c:	79 e2                	jns    40006060 <__lock+0x30>
    4000607e:	8d 8a ff ff ff 7f    	lea    0x7fffffff(%rdx),%ecx
    40006084:	89 d6                	mov    %edx,%esi
    40006086:	eb e0                	jmp    40006068 <__lock+0x38>
    40006088:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    4000608f:	00 
    40006090:	c3                   	ret    
    40006091:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40006098:	c6 05 04 1d 00 00 00 	movb   $0x0,0x1d04(%rip)        # 40007da3 <__libc+0x3>
    4000609f:	85 c0                	test   %eax,%eax
    400060a1:	75 ad                	jne    40006050 <__lock+0x20>
    400060a3:	eb eb                	jmp    40006090 <__lock+0x60>
    400060a5:	0f 1f 00             	nopl   (%rax)
    400060a8:	53                   	push   %rbx
    400060a9:	41 b8 01 00 00 00    	mov    $0x1,%r8d
    400060af:	f0 44 0f c1 07       	lock xadd %r8d,(%rdi)
    400060b4:	41 b9 ca 00 00 00    	mov    $0xca,%r9d
    400060ba:	41 83 c0 01          	add    $0x1,%r8d
    400060be:	bb 80 00 00 00       	mov    $0x80,%ebx
    400060c3:	eb 18                	jmp    400060dd <__lock+0xad>
    400060c5:	0f 1f 00             	nopl   (%rax)
    400060c8:	44 8d 82 00 00 00 80 	lea    -0x80000000(%rdx),%r8d
    400060cf:	89 d0                	mov    %edx,%eax
    400060d1:	f0 44 0f b1 07       	lock cmpxchg %r8d,(%rdi)
    400060d6:	41 89 c0             	mov    %eax,%r8d
    400060d9:	39 c2                	cmp    %eax,%edx
    400060db:	74 33                	je     40006110 <__lock+0xe0>
    400060dd:	44 89 c2             	mov    %r8d,%edx
    400060e0:	45 85 c0             	test   %r8d,%r8d
    400060e3:	79 e3                	jns    400060c8 <__lock+0x98>
    400060e5:	49 63 d0             	movslq %r8d,%rdx
    400060e8:	45 31 d2             	xor    %r10d,%r10d
    400060eb:	4c 89 c8             	mov    %r9,%rax
    400060ee:	48 89 de             	mov    %rbx,%rsi
    400060f1:	0f 05                	syscall 
    400060f3:	48 83 f8 da          	cmp    $0xffffffffffffffda,%rax
    400060f7:	75 07                	jne    40006100 <__lock+0xd0>
    400060f9:	31 f6                	xor    %esi,%esi
    400060fb:	4c 89 c8             	mov    %r9,%rax
    400060fe:	0f 05                	syscall 
    40006100:	41 8d 90 ff ff ff 7f 	lea    0x7fffffff(%r8),%edx
    40006107:	eb bf                	jmp    400060c8 <__lock+0x98>
    40006109:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    40006110:	5b                   	pop    %rbx
    40006111:	c3                   	ret    
    40006112:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    40006119:	00 00 00 
    4000611c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000040006120 <__unlock>:
    40006120:	8b 07                	mov    (%rdi),%eax
    40006122:	85 c0                	test   %eax,%eax
    40006124:	78 0a                	js     40006130 <__unlock+0x10>
    40006126:	c3                   	ret    
    40006127:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    4000612e:	00 00 
    40006130:	b8 ff ff ff 7f       	mov    $0x7fffffff,%eax
    40006135:	f0 0f c1 07          	lock xadd %eax,(%rdi)
    40006139:	3d 01 00 00 80       	cmp    $0x80000001,%eax
    4000613e:	74 e6                	je     40006126 <__unlock+0x6>
    40006140:	41 b8 ca 00 00 00    	mov    $0xca,%r8d
    40006146:	ba 01 00 00 00       	mov    $0x1,%edx
    4000614b:	be 81 00 00 00       	mov    $0x81,%esi
    40006150:	4c 89 c0             	mov    %r8,%rax
    40006153:	0f 05                	syscall 
    40006155:	48 83 f8 da          	cmp    $0xffffffffffffffda,%rax
    40006159:	75 cb                	jne    40006126 <__unlock+0x6>
    4000615b:	4c 89 c0             	mov    %r8,%rax
    4000615e:	48 89 d6             	mov    %rdx,%rsi
    40006161:	0f 05                	syscall 
    40006163:	c3                   	ret    

Disassembly of section .init:

0000000040006164 <_init>:
    40006164:	50                   	push   %rax
    40006165:	58                   	pop    %rax
    40006166:	c3                   	ret    

Disassembly of section .fini:

0000000040006167 <_fini>:
    40006167:	50                   	push   %rax
    40006168:	58                   	pop    %rax
    40006169:	c3                   	ret    
